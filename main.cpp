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
    const char window_title[] = "Target Practie OpenGL Game";
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
    glfwSwapInterval(1); //TODO settings for v-sync
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

    //initializing shared gl context
    const glm::ivec2 window_fbo_size = WindowManager::getFBOSize();
    //TODO consider not using fbo with OpenGLES 2.0, as it forces very limited depth resolution
    bool use_fbo = true; //TODO add settings option for this (probably when postprocessing is off?)

    assert(!SharedGLContext::instance.has_value());
    SharedGLContext& sharedGLContext = SharedGLContext::instance.emplace(use_fbo, window_fbo_size.x, window_fbo_size.y);
    if (!sharedGLContext.isInitialized())
    {
        fprintf(stderr, "Failed to initialize shared GL context!\n");
        glfwTerminate();
        return 4;
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

    MainLoopStack main_loop_stack{};
    if (!main_loop_stack.pushFromTemplate<GameMainLoop>())
    {
        fprintf(stderr, "Failed to create wanted LoopData! Most likely out of memory.\n");
        deinit();
        return -1;
    }

    int init_result = main_loop_stack.currentLoopData()->init();
    if (init_result)
    {
        fprintf(stderr, "Failed to initialize wanted Main Loop! Error value: %d\n", init_result);
        deinit();
        return -2;
    }

    //main loop
    {
        const LoopData* loop_data = NULL;
        while(!glfwWindowShouldClose(window) && (loop_data = main_loop_stack.currentLoopData()) != NULL)
        {
            glfwPollEvents();
            loop_data->loopCallback(); //TODO retval
            glfwSwapBuffers(window);
        }
    }

    //deinitialization
    deinit();

    puts("End main.");
    return 0;
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
        MainLoopStack* main_loop_stack = reinterpret_cast<MainLoopStack*>(arg);
        if (!main_loop_stack) return;

        const LoopData *loop_data = main_loop_stack->currentLoopData();
        if (!loop_data) return; //TODO maybe swap buffers anyways?

        GLFWwindow *window = WindowManager::getWindow();
        assert(window != NULL); //TODO error?

        glfwPollEvents();
        loop_data->loopCallback(); //TODO retval
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

    MainLoopStack main_loop_stack{};
    if (!main_loop_stack.pushFromTemplate<GameMainLoop>())
    {
        fprintf(stderr, "Failed to create wanted LoopData! Most likely out of memory.\n");
        deinit();
        return -1;
    }

    int init_result = main_loop_stack.currentLoopData()->init();
    if (init_result)
    {
        fprintf(stderr, "Failed to initialize wanted Main Loop! Error value: %d\n", init_result);
        deinit();
        return -2;
    }

    emscripten_set_main_loop_arg(web_loop, reinterpret_cast<void*>(&main_loop_stack), 0, true);

    //deinitialization
    deinit();

    puts("web_main end");
    return 0;
}
#endif /* PLATFORM_WEB */

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
