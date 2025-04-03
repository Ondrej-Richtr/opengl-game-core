#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "game.hpp"
#include "stb_image.h"

#ifdef PLATFORM_WEB
    #include <emscripten/emscripten.h>
#endif

static int init(void)
{
    puts("Setup begin.");

    //GLFW initialization
    if (glfwInit() == GLFW_FALSE)
    {
        fprintf(stderr, "GLFW failed to initialize!\n");
        return 1;
    }

    //setting up stbi
    stbi_set_flip_vertically_on_load(true);

    //setting up OpenGL in GLFW
    #ifdef BUILD_OPENGL_330_CORE
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // ignored for OpenGL ES
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           // Mac OS X only, ignored for OpenGL ES
        glfwWindowHint(GLFW_SCALE_FRAMEBUFFER, GL_FALSE);
        //TODO look up GLFW_COCOA_RETINA_FRAMEBUFFER
        //TODO GLFW_SCALE_FRAMEBUFFER
        //TODO GLFW_SCALE_TO_MONITOR
    #else
        //ES
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    #endif

    //initializing the window
    const char window_title[] = "OpenGL Game";
    // GLFWwindow* window = glfwCreateWindow(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, (char*)window_title, glfwGetPrimaryMonitor(), NULL);
    GLFWwindow* window = glfwCreateWindow(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, (char*)window_title, NULL, NULL);
    if (window == NULL)
    {
        fprintf(stderr, "GLFW failed to create window!\n");
        glfwTerminate();
        return 2;
    }
    glfwMakeContextCurrent(window);
    
    WindowManager::init(window);

    //other GLFW settings
    glfwSwapInterval(1); //TODO check this, maybe 0?
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); //DEBUG
    // try to enable raw mouse motion, only takes effect when the cursor is disabled
    if (glfwRawMouseMotionSupported()) glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    else fprintf(stderr, "[WARNING] Failed to enable raw mouse motion.\n");


    //initializing GLAD
    #ifdef BUILD_OPENGL_330_CORE
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            fprintf(stderr, "GLAD failed to initialize OpenGL 3.3 core!\n");
            glfwTerminate();
            return 3;
        }
    #else
        //ES
        if (!gladLoadGLES2Loader((GLADloadproc)glfwGetProcAddress))
        {
            fprintf(stderr, "GLAD failed to initialize OpenGLES 2.0!\n");
            glfwTerminate();
            return 3;
        }
    #endif

    //Inits
    if (!Meshes::initBasicMeshes())
    {
        fprintf(stderr, "Failed to initialize basic meshes!\n");
        glfwTerminate();
        return 4;
    }

    const glm::ivec2 window_fbo_size = WindowManager::getFBOSize();
    //TODO consider not using fbo with OpenGLES 2.0, as it forces very limited depth resolution
    bool use_fbo = true;

    assert(!SharedGLContext::instance.has_value());
    SharedGLContext& sharedGLContext = SharedGLContext::instance.emplace(use_fbo, window_fbo_size.x, window_fbo_size.y);
    if (!sharedGLContext.isInitialized())
    {
        fprintf(stderr, "Failed to initialize shared GL context!\n");
        glfwTerminate();
        return 5;
    }

    puts("Setup end.");
    return 0;
}

static void deinit()
{
    glfwTerminate();
}

int desktop_main(void)
{
    int setup_ret = init();
    if (setup_ret)
    {
        fprintf(stderr, "Setup failed with value: %d\n", setup_ret);
        return 1;
    }

    puts("Begin main.");

    GLFWwindow *window = WindowManager::getWindow();
    assert(window != NULL);

    // Creating struct this way so it's fields won't get initialized before init method call
    unsigned char loop_memory[sizeof(GameMainLoop)];
    //TODO check if optimizer optimizes this and omits pointless dereference
    GameMainLoop& loop = *reinterpret_cast<GameMainLoop*>(&loop_memory); //TODO better cast
    int result = loop.init();
    if (result)
    {
        fprintf(stderr, "Failed to initialize wanted Main Loop! Error value: %d\n", result);
        deinit();
        return result;
    }

    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        loop.loop(); //TODO retval
        glfwSwapBuffers(window);
    }

    //destructors
    loop.~GameMainLoop();

    deinit();
    puts("End main.");
    return result;
}

#ifdef PLATFORM_WEB
extern "C"
{
    void emsc_set_window_size(int width, int height)
    {
        //TODO
        printf("emsc_set_window_size called with: %dx%d\n", width, height);

        GLFWwindow *window = WindowManager::getWindow();
        if (window != NULL)
        {
            WindowManager::windowResizeCallback(window, width, height);
            WindowManager::framebufferResizeCallback(window, width, height);
        }
    }

    void web_loop(void *arg)
    {
        GameMainLoop& loop = *reinterpret_cast<GameMainLoop*>(arg); //TODO better cast
        GLFWwindow *window = WindowManager::getWindow();
        assert(window != NULL); //TODO error?

        glfwPollEvents();
        loop.loop(); //TODO retval
        glfwSwapBuffers(window);
    }
}

int web_main()
{
    puts("web_main begin");

    int setup_ret = init();
    if (setup_ret)
    {
        fprintf(stderr, "Setup failed with value: %d\n", setup_ret);
        return 1;
    }

    puts("Begin main.");

    // Creating struct this way so it's fields won't get initialized before init method call
    unsigned char loop_memory[sizeof(GameMainLoop)];
    //TODO check if optimizer optimizes this and omits pointless dereference
    GameMainLoop& loop = *reinterpret_cast<GameMainLoop*>(&loop_memory); //TODO better cast
    int result = loop.init();
    if (result)
    {
        fprintf(stderr, "Failed to initialize wanted Main Loop! Error value: %d\n", result);
        deinit();
        return result;
    }

    emscripten_set_main_loop_arg(web_loop, reinterpret_cast<void*>(&loop), 0, true);

    //destructors
    loop.~GameMainLoop();

    deinit();
    puts("web_main end");
    return result;
    // return 0;
}
#endif

int main()
{
    #ifdef BUILD_OPENGL_330_CORE
        puts("[MAIN] Build with OpenGL 3.3");
    #else
        puts("[MAIN] Build with OpenGL ES 2.0");
    #endif

    #ifdef USE_VER100_SHADERS
        puts("[MAIN] Using GLSL shader version 100");
    #else
        puts("[MAIN] Using GLSL shader version 330 core");
    #endif

    #ifdef PLATFORM_WEB
        return web_main();
    #else
        return desktop_main();
    #endif
}
