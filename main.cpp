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
    //ES
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // ignored for OpenGL ES
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           // Mac OS X only, ignored for OpenGL ES

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
    //glfwSetCursorPosCallback(window, windowMouseMoveCallback); 

    //initializing GLAD
    if (!gladLoadGLES2Loader((GLADloadproc)glfwGetProcAddress))
    {
        fprintf(stderr, "GLAD failed to initialize!\n");
        glfwTerminate();
        return 3;
    }

    //Inits
    if (!Meshes::initBasicMeshes())
    {
        fprintf(stderr, "Failed to initialize basic meshes!\n");
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

int main(void)
{
    // return game_main();
    // return test_main();

    int setup_ret = init();
    if (setup_ret)
    {
        fprintf(stderr, "Setup failed with value: %d\n", setup_ret);
        return 1;
    }

    //DEBUG
    puts("Begin main.");

    GLFWwindow *window = WindowManager::getWindow();
    assert(window != NULL);

    // Creating struct this way so it's fields won't get initialized before init method call
    unsigned char loop_memory[sizeof(TestMainLoop)];
    //TODO check if compile optimizes this and omits pointless dereference
    TestMainLoop& loop = *reinterpret_cast<TestMainLoop*>(&loop_memory); //TODO better cast
    int result = loop.init();
    if (result) return result;

    while(!glfwWindowShouldClose(window))
    {
        loop.loop(); //TODO retval
    }

    //destructors
    loop.~TestMainLoop();

    deinit();
    puts("End main.");
    return result;
}
