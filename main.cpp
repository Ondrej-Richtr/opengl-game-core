#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "game.hpp"
#include "stb_image.h"


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

    //TODO remove this
    // glGenVertexArrays(1, &default_vao);
    // glBindVertexArray(default_vao);

    puts("Setup end.");
    return 0;
}

static void deinit()
{
    glfwTerminate();
}

int main(void)
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
