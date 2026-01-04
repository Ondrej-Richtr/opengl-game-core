#include "game.hpp"
#include "stb_image.h"

#include <cstring>


bool GamePauseMainLoop::initTextures()
{
    const SharedGLContext& shared_gl_context = SharedGLContext::instance.value();
    const Drawing::FrameBuffer& fbo3d_conv = shared_gl_context.getFbo3D(true);
    const glm::ivec2 fbo3d_conv_size = shared_gl_context.getFbo3DSize(true);

    //Background texture from fbo3d
    new (&background_tex) Textures::Texture2D(fbo3d_conv_size.x, fbo3d_conv_size.y, GL_RGB);
    if (background_tex.m_id == empty_id)
    {
        fprintf(stderr, "[WARNING] Failed to initialize Texture for background of pause menu!\n");
        background_tex.~Texture2D();
        // No return!!! We can cope with uninitialized background texture.
    }
    // copy contents of `fbo3d_conv_tex` into `background_tex`
    else if (!background_tex.copyContentsFrom(fbo3d_conv, background_tex.m_width, background_tex.m_height, GL_RGB))
    {
        fprintf(stderr, "[WARNING] Failed to copy data of FrameBuffer into pause menu background Texture!\n");
        background_tex.~Texture2D();
        // No return!!! We can cope with uninitialized background texture.
    }

    return true;
}

void GamePauseMainLoop::deinitTextures()
{
    background_tex.~Texture2D();
}

bool GamePauseMainLoop::initShaders()
{
    //shader partials
    using ShaderInclude = Shaders::ShaderInclude;
    using IncludeDefine = Shaders::IncludeDefine;

    // used only during this init method, thus loaded as local unique_ptr, so we dont have to call delete/destructor
    const char *postprocess_fs_partial_path = SHADERS_PARTIALS_DIR_PATH "postprocess.fspart";
    std::unique_ptr<char[]> postprocess_fs_partial = Utils::getTextFileAsString(postprocess_fs_partial_path, NULL);
    if (!postprocess_fs_partial)
    {
        fprintf(stderr, "Failed to load postprocessing fragment shader partial file: '%s'!\n", postprocess_fs_partial_path);
        return false;
    }

    using ShaderP = Shaders::Program;

    const char // *default_vs_path = SHADERS_DIR_PATH "default.vs",
            //    *default_fs_path = SHADERS_DIR_PATH "default.fs",
            //    *passthrough_pos_vs_path = SHADERS_DIR_PATH "passthrough-pos.vs",
            //    *passthrough_pos_uv_vs_path = SHADERS_DIR_PATH "passthrough-pos-uv.vs",
               *transform_vs_path = SHADERS_DIR_PATH "transform.vs",
               *static_color_fs_path = SHADERS_DIR_PATH "static-color.fs";

    //line shader
    const char *screen_line_vs_path = SHADERS_DIR_PATH "screen2d-line.vs";

    new (&screen_line_shader) ShaderP(screen_line_vs_path, static_color_fs_path);
    if (screen_line_shader.m_id == empty_id)
    {
        fprintf(stderr, "Failed to create screen line shader program!\n");
        screen_line_shader.~Program();
        return false;
    }

    //ui shader
    const char *ui_vs_path = SHADERS_DIR_PATH "ui.vs",
               *ui_fs_path = SHADERS_DIR_PATH "ui.fs";
    std::vector<ShaderInclude> ui_vs_includes = {},
                               ui_fs_includes = {};
    
    new (&ui_shader) ShaderP(ui_vs_path, ui_fs_path, ui_vs_includes, ui_fs_includes);
    if (ui_shader.m_id == empty_id)
    {
        fprintf(stderr, "Failed to create UI shader program!\n");
        screen_line_shader.~Program();
        ui_shader.~Program();
        return false;
    }

    //textured rectangle shader
    const char *tex_rect_fs_path = SHADERS_DIR_PATH "tex-rect.fs";
    std::vector<ShaderInclude> gray_tex_rect_vs_includes{},
                               gray_tex_rect_fs_includes = {
                                                            ShaderInclude(IncludeDefine("DITHER_ON_COLOR",  "(vec4(0.4, 0.4, 0.4, 1.0))")), // set the dither "on" color to darker gray
                                                            ShaderInclude(IncludeDefine("DITHER_OFF_COLOR", "(vec4(0.0, 0.0, 0.0, 1.0))")), // rest of dither is black
                                                            ShaderInclude(IncludeDefine("POSTPROCESS(tex, tpos)", "(_postproc_dither_gray_mix((tex), (tpos)))")),
                                                            ShaderInclude(postprocess_fs_partial.get()),
                                                           };

    new (&gray_tex_rect_shader) ShaderP(transform_vs_path, tex_rect_fs_path, gray_tex_rect_vs_includes, gray_tex_rect_fs_includes);
    if (gray_tex_rect_shader.m_id == empty_id)
    {
        fprintf(stderr, "Failed to create textured rectangle shader program!\n");
        screen_line_shader.~Program();
        ui_shader.~Program();
        gray_tex_rect_shader.~Program();
        return false;
    }

    return true;
}

void GamePauseMainLoop::deinitShaders()
{
    screen_line_shader.~Program();
    ui_shader.~Program();
    gray_tex_rect_shader.~Program();
}

bool GamePauseMainLoop::initUI()
{
    const char *font_path = "assets/DINEngschrift-Regular.ttf";
    const float font_size = 22;

    //TODO load stuff into textbuffer
    memset(textbuffer, 0, sizeof(textbuffer));
    textbuffer_len = 0;

    new (&font) UI::Font(font_path, font_size);
    if (font.getFontPtr() == NULL)
    {
        fprintf(stderr, "Failed to initialize UI font!\n");
        font.~Font();
        return false;
    }

    new (&ui) UI::Context(ui_shader, font);
    if (!ui.m_ctx_initialized)
    {
        fprintf(stderr, "Failed to initialize UI!\n");
        font.~Font();
        ui.~Context();
        return false;
    }

    return true;
}
void GamePauseMainLoop::deinitUI()
{
    font.~Font();
    ui.~Context();
}

int GamePauseMainLoop::init()
{
    //Textures
    if (!initTextures())
    {
        return 1;
    }

    //Shaders
    if (!initShaders())
    {
        deinitTextures();
        return 2;
    }

    //UI
    if (!initUI())
    {
        deinitTextures();
        deinitShaders();
        return 3;
    }

    //Misc.
    clear_color = Color(0, 0, 0);
    tick = 0;
    last_global_tick = 0;
    last_esc_state = GLFW_PRESS;

    return 0;
}

GamePauseMainLoop::~GamePauseMainLoop()
{
    // nothing for now
}

LoopRetVal GamePauseMainLoop::loop(unsigned int global_tick, double frame_time, float frame_delta)
{
    GLFWwindow * const window = WindowManager::getWindow();
    const glm::vec2 win_size = WindowManager::getSizeF();
    SharedGLContext& shared_gl_context = SharedGLContext::instance.value();
    assert(shared_gl_context.isInitialized());

    const bool consecutive_tick = (last_global_tick + 1) == global_tick;

    // ---Mouse input---
    if (!consecutive_tick) MouseManager::setCursorVisible();

    const glm::vec2 mouse_posF = MouseManager::mousePosF();

    const bool left_mbutton = MouseManager::left_button, right_mbutton = MouseManager::right_button;
    // const bool left_mbutton_is_clicked = left_mbutton && !last_left_mbutton, right_mbutton_is_clicked = right_mbutton && !last_right_mbutton;

    // ---Keyboard input---
    if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) // exit on Q
    {
        return LoopRetVal::exit;
    }

    const int esc_state = glfwGetKey(window, GLFW_KEY_ESCAPE);
    const bool esc_clicked = consecutive_tick && esc_state == GLFW_PRESS && last_esc_state == GLFW_RELEASE;
    if (esc_clicked) // go back to the game on ESC
    {
        return LoopRetVal::popTop;
    }

    // ---UI---
    //pump the input into UI
    if (!ui.getInput(window, mouse_posF, left_mbutton, textbuffer, textbuffer_len))
    {
        fprintf(stderr, "[WARNING] Failed to update the input for UI!\n");
    }

    //GUI styling    
    {
        nk_color ui_background_color = nk_rgba(200, 200, 80, 200);
        ui.m_ctx.style.window.background = ui_background_color;
        ui.m_ctx.style.window.fixed_background = nk_style_item_color(ui_background_color);
        ui.m_ctx.style.window.border_color = nk_rgb(100, 100, 80);
        ui.m_ctx.style.window.border = 3;
        ui.m_ctx.style.window.padding = nk_vec2(8, 4);
        ui.m_ctx.style.text.color = nk_rgb(0, 0, 0);
    }
    //GUI definition+logic
    {
        //TODO change this probably
        char ui_textbuff[256]{};
        size_t ui_textbuff_capacity = sizeof(ui_textbuff) / sizeof(ui_textbuff[0]); // including term. char.

        const glm::vec2 menu_size(200, 250);
        
        //Menu
        if (nk_begin(&ui.m_ctx, "Paused", nk_rect((win_size.x - menu_size.x) / 2.f, (win_size.y - menu_size.y) / 2.f,
                                                menu_size.x, menu_size.y),
            NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_NO_SCROLLBAR))
        {
            ui.verticalGap(12.f);

            nk_layout_row_dynamic(&ui.m_ctx, 40, 1);
            if(nk_button_label(&ui.m_ctx, "Resume [ESC]"))
            {
                nk_end(&ui.m_ctx);
                nk_clear(&ui.m_ctx);
                return LoopRetVal::popTop;
            }

            ui.verticalGap(12.f);

            nk_layout_row_dynamic(&ui.m_ctx, 40, 1);
            if(nk_button_label(&ui.m_ctx, "Options"))
            {
                MainLoopStack& main_loop_stack = MainLoopStack::instance;

                LoopData* options_loop = main_loop_stack.pushFromTemplate<GameOptionsMainLoop>();
                if (options_loop == NULL)
                {
                    fprintf(stderr, "Failed to open the options menu! Can't push options main loop on the stack.\n");
                }
                else
                {
                    GameOptionsMainLoop* game_options_main_loop = static_cast<GameOptionsMainLoop*>(options_loop->getData());
                    assert(game_options_main_loop != NULL);

                    //parameter passing
                    game_options_main_loop->setParameters(background_tex, ui_shader, gray_tex_rect_shader, ui);

                    //initialization
                    int init_result = options_loop->init();
                    if (init_result)
                    {
                        fprintf(stderr, "Failed to open the options menu! Initialization failed with return value: %d.\n", init_result);
                    }
                }

                //return from this loop early
                nk_end(&ui.m_ctx);
                nk_clear(&ui.m_ctx);
                return LoopRetVal::ok;
            }

            ui.verticalGap(20.f);

            nk_layout_row_dynamic(&ui.m_ctx, 40, 1);
            if(nk_button_label(&ui.m_ctx, "Quit [Q]"))
            {
                nk_end(&ui.m_ctx);
                nk_clear(&ui.m_ctx);
                return LoopRetVal::exit;
            }
        }
        nk_end(&ui.m_ctx);
    }

    // Credits 
    {
        ui.m_ctx.style.window.background = nk_rgba(0, 0, 0, 0);
        ui.m_ctx.style.window.fixed_background = nk_style_item_color(nk_rgba(0, 0, 0, 0));
        ui.m_ctx.style.text.color = nk_rgba(0, 0, 0, 100);
        ui.m_ctx.style.window.padding = nk_vec2(3, 3);
    }
    const glm::vec2 credits_size(156, 56);
    if (nk_begin(&ui.m_ctx, "Credits", nk_rect(win_size.x - credits_size.x, win_size.y - credits_size.y,
                                               credits_size.x, credits_size.y),
        NK_WINDOW_NO_SCROLLBAR))
    {
        nk_layout_row_dynamic(&ui.m_ctx, 25, 1);
        #ifdef VERSION_STRING
            nk_label(&ui.m_ctx, VERSION_STRING, NK_TEXT_RIGHT);
        #else
            ui.verticalGap(25.f);
        #endif
        nk_label(&ui.m_ctx, "Ondrej Richtr, 2025", NK_TEXT_RIGHT);
    }
    nk_end(&ui.m_ctx);

    // ---Drawing---
    {
        bool use_msaa = shared_gl_context.use_msaa;

        //2D block
        {
            //TODO use correct win size + check whether some functions need it as parameter
            const glm::vec2 win_fbo_size = WindowManager::getFBOSizeF();
            const glm::ivec2 win_fbo_size_i = WindowManager::getFBOSize();

            //TODO this might be wrong on some displays?
            //set the viewport according to window size
            glViewport(0, 0, win_fbo_size_i.x, win_fbo_size_i.y);

            //bind the default framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, empty_id);
            Drawing::clear(clear_color);

            glDepthMask(GL_FALSE);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glEnable(GL_BLEND); //TODO check this
            glBlendEquation(GL_FUNC_ADD); //TODO check this
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //TODO check this

            // enable multisampling (only for OpenGL 3.3, as OpenGL ES 2.0 and WebGL1 does not support it)
            #ifdef BUILD_OPENGL_330_CORE
                if (use_msaa) glEnable(GL_MULTISAMPLE);
                else          glDisable(GL_MULTISAMPLE);
            #endif
            
            //render the background texture with gray postprocessing (should be last fbo3d render)
            Drawing::texturedRectangle(gray_tex_rect_shader, background_tex, win_fbo_size, glm::vec2(0.f), win_fbo_size);
            
            //line test
            // Drawing::screenLine(screen_line_shader, line_vbo, win_size,
            //                     window_middle, glm::vec2(50.f),
            //                     50.f, ColorF(1.0f, 0.0f, 0.0f));

            //UI drawing
            glEnable(GL_SCISSOR_TEST); // enable scissor for UI drawing only
            if (!ui.draw(win_fbo_size))
            {
                fprintf(stderr, "[WARNING] Failed to draw the UI!\n");
            }
            glDisable(GL_SCISSOR_TEST);

            assert(!Utils::checkForGLErrorsAndPrintThem()); //DEBUG
        }
    }
    
    ui.clear(); // UI clear is here as we want to call it each frame regardless of drawing stage

    last_esc_state = esc_state;
    last_global_tick = global_tick;
    ++tick;

    return LoopRetVal::ok;
}

void GameOptionsMainLoop::setParameters(Textures::Texture2D& background_tex, Shaders::Program& ui_shader,
                                        Shaders::Program& tex_rect_shader, UI::Context& ui)
{
    ref_background_tex = &background_tex;
    ref_ui_shader = &ui_shader;
    ref_gray_tex_rect_shader = &tex_rect_shader;
    ref_ui = &ui;
}

bool GameOptionsMainLoop::initUI()
{
    //TODO load stuff into textbuffer
    memset(textbuffer, 0, sizeof(textbuffer));
    textbuffer_len = 0;

    return true;
}

void GameOptionsMainLoop::deinitUI()
{
    // nothing for now
}

int GameOptionsMainLoop::init()
{
    //Assert parameters
    assert(ref_ui_shader != NULL);
    assert(ref_gray_tex_rect_shader != NULL);
    assert(ref_ui != NULL);

    if (!initUI())
    {
        return 1;
    }

    //Misc.
    clear_color = Color(0, 0, 0);
    tick = 0;
    last_global_tick = 0;
    last_esc_state = GLFW_PRESS;

    return 0;
}

GameOptionsMainLoop::~GameOptionsMainLoop()
{
    // nothing for now
}

LoopRetVal GameOptionsMainLoop::loop(unsigned int global_tick, double frame_time, float frame_delta)
{
    GLFWwindow * const window = WindowManager::getWindow();
    const glm::vec2 win_size = WindowManager::getSizeF();
    SharedGLContext& shared_gl_context = SharedGLContext::instance.value();
    assert(shared_gl_context.isInitialized());

    const bool consecutive_tick = (last_global_tick + 1) == global_tick;

    // ---Mouse input---
    if (!consecutive_tick) MouseManager::setCursorVisible();

    const glm::vec2 mouse_posF = MouseManager::mousePosF();

    const bool left_mbutton = MouseManager::left_button, right_mbutton = MouseManager::right_button;
    // const bool left_mbutton_is_clicked = left_mbutton && !last_left_mbutton, right_mbutton_is_clicked = right_mbutton && !last_right_mbutton;

    // ---Keyboard input---
    const int esc_state = glfwGetKey(window, GLFW_KEY_ESCAPE);
    const bool esc_clicked = consecutive_tick && esc_state == GLFW_PRESS && last_esc_state == GLFW_RELEASE;
    if (esc_clicked) // go back to the Pause game menu on ESC
    {
        return LoopRetVal::popTop;
    }

    // ---UI---
    UI::Context& ui = *ref_ui;
    
    //pump the input into UI
    if (!ui.getInput(window, mouse_posF, left_mbutton, textbuffer, textbuffer_len))
    {
        fprintf(stderr, "[WARNING] Failed to update the input for UI!\n");
    }

    //GUI styling    
    {
        nk_color ui_background_color = nk_rgba(200, 200, 80, 200);
        ui.m_ctx.style.window.background = ui_background_color;
        ui.m_ctx.style.window.fixed_background = nk_style_item_color(ui_background_color);
        ui.m_ctx.style.window.border_color = nk_rgb(100, 100, 80);
        ui.m_ctx.style.window.border = 3;
        ui.m_ctx.style.window.padding = nk_vec2(8, 4);
        ui.m_ctx.style.text.color = nk_rgb(0, 0, 0);

        ui.m_ctx.style.option.text_normal = nk_rgb(0, 0, 0);
        ui.m_ctx.style.option.text_active = nk_rgb(0, 0, 0);
        ui.m_ctx.style.option.text_hover = nk_rgb(25, 25, 25);

        ui.m_ctx.style.checkbox.text_normal = nk_rgb(0, 0, 0);
        ui.m_ctx.style.checkbox.text_active = nk_rgb(0, 0, 0);
        ui.m_ctx.style.checkbox.text_hover = nk_rgb(25, 25, 25);

        ui.m_ctx.style.option.disabled_factor = 1.f;
    }
    //GUI definition+logic
    {
        //TODO change this probably
        char ui_textbuff[256]{};
        size_t ui_textbuff_capacity = sizeof(ui_textbuff) / sizeof(ui_textbuff[0]); // including term. char.

        const glm::vec2 menu_size(300, 450);
        
        //Menu
        if (nk_begin(&ui.m_ctx, "Options", nk_rect((win_size.x - menu_size.x) / 2.f, (win_size.y - menu_size.y) / 2.f,
                                                   menu_size.x, menu_size.y),
            NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_NO_SCROLLBAR))
        {
            nk_layout_row_dynamic(&ui.m_ctx, 50, 1);
            nk_label_wrap(&ui.m_ctx, "Note that the effect of these changes is visible ingame only");

            ui.verticalGap(5.f);

            //FBO usage
            nk_layout_row_dynamic(&ui.m_ctx, 20, 1);
            nk_label(&ui.m_ctx, "3D", NK_TEXT_LEFT);

            nk_layout_row_dynamic(&ui.m_ctx, 25, 1);
            if (nk_option_label(&ui.m_ctx, "Implicit framebuffer", !shared_gl_context.use_fbo3d))
            {
                shared_gl_context.use_fbo3d = false;
            }
            if (nk_option_label(&ui.m_ctx, "Custom framebuffer", shared_gl_context.use_fbo3d))
            {
                shared_gl_context.use_fbo3d = true;
            }

            ui.verticalGap(12.f);

            //Anti-aliasing
            nk_layout_row_dynamic(&ui.m_ctx, 20, 1);
            nk_label(&ui.m_ctx, "Anti-aliasing", NK_TEXT_LEFT);

            bool antialiasing_enabled = false;
            #ifdef BUILD_OPENGL_330_CORE
                antialiasing_enabled = true;
            #endif

            if (antialiasing_enabled)
            {
                nk_layout_row_dynamic(&ui.m_ctx, 25.f, 1);

                if (nk_option_label(&ui.m_ctx, "None", !shared_gl_context.use_msaa))
                {
                    shared_gl_context.use_msaa = false;
                }
                if (nk_option_label(&ui.m_ctx, "MSAA", shared_gl_context.use_msaa))
                {
                    shared_gl_context.use_msaa = true;
                }

                ui.verticalGap(12.f);
            }
            else
            {
                nk_layout_row_dynamic(&ui.m_ctx, 70.f, 1);
                nk_label_colored_wrap(&ui.m_ctx, "This option is available only for desktop!"
                                                 " However the 'Implicit framebuffer' usually uses MSAA 4x by default.",
                                      nk_rgb(200, 50, 50));

                ui.verticalGap(3.f);
            }

            //Gamma
            nk_layout_row_dynamic(&ui.m_ctx, 20, 1);

            nk_bool gamma_enabled = shared_gl_context.enable_gamma_correction ? nk_true : nk_false;
            if (nk_checkbox_label_align(&ui.m_ctx, "Gamma correction", &gamma_enabled, NK_WIDGET_RIGHT, NK_TEXT_LEFT))
            {
                shared_gl_context.enable_gamma_correction = !shared_gl_context.enable_gamma_correction;
            }

            if (shared_gl_context.enable_gamma_correction)
            {
                const float gamma_label_max_size = 30.f;

                nk_layout_row_begin(&ui.m_ctx, NK_STATIC, 25.f, 2);

                nk_layout_row_push(&ui.m_ctx, gamma_label_max_size);
                snprintf(ui_textbuff, ui_textbuff_capacity, "%.2f", shared_gl_context.gamma_coef);
                nk_label(&ui.m_ctx, ui_textbuff, NK_TEXT_LEFT);

                //TODO add small buttons to change gamma value by small steps (nk_button_image)

                nk_layout_row_push(&ui.m_ctx, menu_size.x - gamma_label_max_size - 24.f);
                const float min_gamma_coef = 0.05f, max_gamma_coef = 10.f, gamma_coef_step = 0.01f;
                float new_gamma_coef = std::max(std::min(shared_gl_context.gamma_coef, max_gamma_coef), min_gamma_coef);
                if (nk_slider_float(&ui.m_ctx, min_gamma_coef, &new_gamma_coef, max_gamma_coef, gamma_coef_step))
                {
                    // dont change the value unless the slider was interacted with
                    if (new_gamma_coef != shared_gl_context.gamma_coef)
                    {
                        shared_gl_context.gamma_coef = new_gamma_coef;
                    }
                }

                nk_layout_row_end(&ui.m_ctx);
            }

            ui.verticalGap(28.f);

            nk_layout_row_dynamic(&ui.m_ctx, 40, 1);
            if(nk_button_label(&ui.m_ctx, "Back [ESC]"))
            {
                nk_end(&ui.m_ctx);
                nk_clear(&ui.m_ctx);
                return LoopRetVal::popTop;
            }
        }
        nk_end(&ui.m_ctx);
    }

    // Credits 
    {
        ui.m_ctx.style.window.background = nk_rgba(0, 0, 0, 0);
        ui.m_ctx.style.window.fixed_background = nk_style_item_color(nk_rgba(0, 0, 0, 0));
        ui.m_ctx.style.text.color = nk_rgba(0, 0, 0, 100);
        ui.m_ctx.style.window.padding = nk_vec2(3, 3);
    }
    const glm::vec2 credits_size(156, 56);
    if (nk_begin(&ui.m_ctx, "Credits", nk_rect(win_size.x - credits_size.x, win_size.y - credits_size.y,
                                               credits_size.x, credits_size.y),
        NK_WINDOW_NO_SCROLLBAR))
    {
        nk_layout_row_dynamic(&ui.m_ctx, 25, 1);
        #ifdef VERSION_STRING
            nk_label(&ui.m_ctx, VERSION_STRING, NK_TEXT_RIGHT);
        #else
            ui.verticalGap(25.f);
        #endif
        nk_label(&ui.m_ctx, "Ondrej Richtr, 2025", NK_TEXT_RIGHT);
    }
    nk_end(&ui.m_ctx);

    // ---Drawing---
    {
        bool use_msaa = shared_gl_context.use_msaa;
        
        //2D block
        {
            //TODO use correct win size + check whether some functions need it as parameter
            const glm::vec2 win_fbo_size = WindowManager::getFBOSizeF();
            const glm::ivec2 win_fbo_size_i = WindowManager::getFBOSize();

            //TODO this might be wrong on some displays?
            //set the viewport according to window size
            glViewport(0, 0, win_fbo_size_i.x, win_fbo_size_i.y);

            //bind the default framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, empty_id);
            Drawing::clear(clear_color);

            glDepthMask(GL_FALSE);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glEnable(GL_BLEND); //TODO check this
            glBlendEquation(GL_FUNC_ADD); //TODO check this
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //TODO check this

            // enable multisampling (only for OpenGL 3.3, as OpenGL ES 2.0 and WebGL1 does not support it)
            #ifdef BUILD_OPENGL_330_CORE
                if (use_msaa) glEnable(GL_MULTISAMPLE);
                else          glDisable(GL_MULTISAMPLE);
            #endif
            
            //render the background texture with gray postprocessing (should be last fbo3d render)
            Drawing::texturedRectangle(*ref_gray_tex_rect_shader, *ref_background_tex, win_fbo_size, glm::vec2(0.f), win_fbo_size);

            //UI drawing
            glEnable(GL_SCISSOR_TEST); // enable scissor for UI drawing only
            if (!ui.draw(win_fbo_size))
            {
                fprintf(stderr, "[WARNING] Failed to draw the UI!\n");
            }
            glDisable(GL_SCISSOR_TEST);

            assert(!Utils::checkForGLErrorsAndPrintThem()); //DEBUG
        }
    }
    
    ui.clear(); // UI clear is here as we want to call it each frame regardless of drawing stage

    last_esc_state = esc_state;
    last_global_tick = global_tick;
    ++tick;

    return LoopRetVal::ok;
}
