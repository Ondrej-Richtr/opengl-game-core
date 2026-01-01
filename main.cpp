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

    //set sample count
    const unsigned int glfw_samples = 4;
    glfwWindowHint(GLFW_SAMPLES, glfw_samples);

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

    //initializing mouse manager
    MouseManager::init(window);

    //initializing shared gl context
    const glm::ivec2 window_fbo_size = WindowManager::getFBOSize();
    //TODO consider not using fbo with OpenGLES 2.0, as it forces very limited depth resolution (maybe dont use extarnal FBO ever?)
    bool use_fbo = true; //TODO add settings option for this (probably when postprocessing is off?)
    bool use_msaa = true;
    bool enable_gamma_correction = true;

    // glfw sample hint == 4, fbo samples == 1, enabled MSAA, disabled FBO => anti-aliasing works on every setup
    unsigned int fbo_samples = 1;
    #ifdef BUILD_OPENGL_330_CORE
        fbo_samples = glfw_samples;
    #endif

    assert(!SharedGLContext::instance.has_value());
    SharedGLContext& sharedGLContext = SharedGLContext::instance.emplace(use_fbo, window_fbo_size.x, window_fbo_size.y,
                                                                         fbo_samples, use_msaa, enable_gamma_correction);
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

    MainLoopStack& main_loop_stack = MainLoopStack::instance;
    {
        LoopData *pushed_loop_data = main_loop_stack.pushFromTemplate<GameMainLoop>();
        // LoopData *pushed_loop_data = main_loop_stack.pushFromTemplate<TestMainLoop>(); //DEBUG
        if (pushed_loop_data == NULL)
        {
            fprintf(stderr, "Failed to create wanted LoopData! Most likely out of memory.\n");
            deinit();
            return -1;
        }

        int init_result = pushed_loop_data->init();
        if (init_result)
        {
            fprintf(stderr, "Failed to initialize wanted Main Loop! Error value: %d\n", init_result);
            deinit();
            return -2;
        }
    }

    //main loop
    {
        const LoopData* loop_data = NULL;
        unsigned int global_ticks = 0;
        while(!glfwWindowShouldClose(window) && (loop_data = main_loop_stack.currentLoopData()) != NULL)
        {
            glfwPollEvents();

            const double current_frame_time = glfwGetTime();
            const float frame_delta = main_loop_stack.getFrameDelta(current_frame_time);
            const LoopRetVal loop_ret_val = loop_data->loopCallback(global_ticks, current_frame_time, frame_delta);

            glfwSwapBuffers(window);

            //resolve loop_ret_val
            switch (loop_ret_val)
            {
            case LoopRetVal::ok:
                // continue normally
                break;
            case LoopRetVal::exit:
                // exit
                glfwSetWindowShouldClose(window, true);
                break;
            case LoopRetVal::popTop:
                main_loop_stack.pop();
                break;
            default:
                assert(false); // unimplemented case!
                break;
            }

            ++global_ticks;
        }
    }

    //deinitialization
    deinit();

    puts("End main.");
    return 0;
}

#ifdef PLATFORM_WEB
typedef struct
{
    unsigned int global_ticks;
} WebLoopState;

extern "C"
{
    void emsc_set_window_size(int width, int height)
    {
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
        WebLoopState* state = reinterpret_cast<WebLoopState*>(arg);
        if (!state) return;

        MainLoopStack& main_loop_stack = MainLoopStack::instance;
        unsigned int& global_ticks = state->global_ticks;

        const LoopData *loop_data = main_loop_stack.currentLoopData();
        if (!loop_data) return; //TODO maybe swap buffers anyways?

        GLFWwindow *window = WindowManager::getWindow();
        assert(window != NULL); //TODO error?

        //loop routine
        glfwPollEvents();

        const double current_frame_time = glfwGetTime();
        const float frame_delta = main_loop_stack.getFrameDelta(current_frame_time);
        const LoopRetVal loop_ret_val = loop_data->loopCallback(global_ticks, current_frame_time, frame_delta);

        glfwSwapBuffers(window);

        //resolve loop_ret_val
        switch (loop_ret_val)
        {
        case LoopRetVal::ok:
            // continue normally
            break;
        case LoopRetVal::exit:
            // exit
            glfwSetWindowShouldClose(window, true); //TODO implement exiting on web
            break;
        case LoopRetVal::popTop:
            main_loop_stack.pop();
            break;
        default:
            assert(false); // unimplemented case!
            break;
        }

        ++global_ticks;
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

    MainLoopStack& main_loop_stack = MainLoopStack::instance;
    LoopData *pushed_loop_data = main_loop_stack.pushFromTemplate<GameMainLoop>();
    if (pushed_loop_data == NULL)
    {
        fprintf(stderr, "Failed to create wanted LoopData! Most likely out of memory.\n");
        deinit();
        return -1;
    }

    int init_result = pushed_loop_data->init();
    if (init_result)
    {
        fprintf(stderr, "Failed to initialize wanted Main Loop! Error value: %d\n", init_result);
        deinit();
        return -2;
    }

    WebLoopState state{ 0 };
    emscripten_set_main_loop_arg(web_loop, reinterpret_cast<void*>(&state), 0, true);

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
