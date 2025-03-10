#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "game.hpp"

#include <cstring>


UI::Font::Font(const char *font_path, float font_height)
                : m_font(NULL)
{
    nk_font_atlas_init_default(&m_atlas);
    nk_font_atlas_begin(&m_atlas);

    //struct nk_font_config config = {}; //TODO
    // m_font = nk_font_atlas_add_default(&m_atlas, font_height, NULL); //DEBUG
    m_font = nk_font_atlas_add_from_file(&m_atlas, font_path, font_height, NULL);
    if (m_font == NULL)
    {
        fprintf(stderr, "Could not initialize font from file: '%s'!\n", font_path);

        nk_font_atlas_end(&m_atlas, nk_handle_id(0), NULL); // 0  as placeholder id
        nk_font_atlas_clear(&m_atlas);

        return;
    }

    int img_width = 0, img_height = 0;
    const void* img = nk_font_atlas_bake(&m_atlas, &img_width, &img_height, NK_FONT_ATLAS_RGBA32);
    if (img == NULL || img_width <= 0 || img_height <= 0)
    {
        fprintf(stderr, "Could not bake the font from file: '%s'!\n", font_path);

        m_font = NULL;
        nk_font_atlas_end(&m_atlas, nk_handle_id(0), NULL); // 0  as placeholder id
        nk_font_atlas_clear(&m_atlas);

        return;
    }

    //create the texture from font baked image data
    // we use placement new here as that means we dont have to define move assign operator for Textures::Texture2D
    m_texture.~Texture2D(); // call the destructor just in case
    new(&m_texture) Textures::Texture2D(img, img_width, img_height, false); // false for no mipmaps
    if (m_texture.m_id == Textures::empty_id)
    {
        fprintf(stderr, "Could not create texture from baked font that was loaded from file: '%s'!\n", font_path);

        m_font = NULL;
        nk_font_atlas_end(&m_atlas, nk_handle_id(0), NULL); // 0  as placeholder id
        nk_font_atlas_clear(&m_atlas);

        return;
    }

    //printf("Created font texture with id: %d\n", m_texture.m_id);

    nk_font_atlas_end(&m_atlas, nk_handle_id(static_cast<int>(m_texture.m_id)), &m_null_texture);
}

UI::Font::~Font()
{
    nk_font_atlas_clear(&m_atlas);
}

const nk_user_font* UI::Font::getFontPtr() const
{
    if (!m_font) return NULL;

    return &m_font->handle;
}

UI::Context::Context(const Shaders::Program& shader, const UI::Font& font)
                        : m_ctx_initialized(false), m_shader(shader),
                          m_vbo_id(0), m_ebo_id(0) //TODO empty id
{
    assert(!Utils::checkForGLError());

    const nk_user_font *font_ptr = font.getFontPtr();

    if (!nk_init_default(&m_ctx, font_ptr))
    {
        fprintf(stderr, "Failed to initialize nuklear context!\n");
        return;
    }

    // fill configuration
    memset(&m_cfg, 0, sizeof(nk_convert_config));

    // must be static for pointer to remain valid in m_cfg
    static const struct nk_draw_vertex_layout_element vertex_layout[] =
    {
        { NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(UI::Vertex, pos) },
        { NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(UI::Vertex, uv) },
        { NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(UI::Vertex, color) },
        { NK_VERTEX_LAYOUT_END }
    };

    m_cfg.shape_AA = NK_ANTI_ALIASING_ON;
    m_cfg.line_AA = NK_ANTI_ALIASING_ON;
    m_cfg.vertex_layout = vertex_layout;
    m_cfg.vertex_size = sizeof(UI::Vertex);
    m_cfg.vertex_alignment = NK_ALIGNOF(UI::Vertex);
    m_cfg.circle_segment_count = 22;
    m_cfg.curve_segment_count = 22;
    m_cfg.arc_segment_count = 22;
    m_cfg.global_alpha = 1.0f;
    m_cfg.tex_null = font.m_null_texture;

    //TODO somehow check if error happened
    nk_buffer_init_default(&m_cmd_buffer);
    //IDEA maybe use fixed sized buffers
    nk_buffer_init_default(&m_vert_buffer);
    nk_buffer_init_default(&m_idx_buffer);

    //TODO VBO when USE_VBO macro defined

    GLuint buffer_obj[2];
    glGenBuffers(2, buffer_obj);
    if (Utils::checkForGLError())
    {
        fprintf(stderr, "Failed to generate GL buffers for UI context!\n");

        nk_free(&m_ctx);
        nk_buffer_free(&m_cmd_buffer);
        nk_buffer_free(&m_vert_buffer);
        nk_buffer_free(&m_idx_buffer);

        return;
    }

    m_vbo_id = buffer_obj[0];
    m_ebo_id = buffer_obj[1];

    m_ctx_initialized = true;
}

UI::Context::~Context()
{
    if (!m_ctx_initialized) return;

    glDeleteBuffers(1, &m_vbo_id);
    glDeleteBuffers(1, &m_ebo_id);
    
    nk_free(&m_ctx);

    nk_buffer_free(&m_cmd_buffer);
    nk_buffer_free(&m_vert_buffer);
    nk_buffer_free(&m_idx_buffer);
}

bool UI::Context::getInput(GLFWwindow* window, glm::vec2 mouse_pos, bool mouse_left_is_down,
                           unsigned int textbuffer[], size_t textbuffer_len)
{
    assert(m_ctx_initialized); //DEBUG
    if (!m_ctx_initialized) return false;

    const std::pair<enum nk_keys, int> checked_keys[] =
    {
        { NK_KEY_SHIFT, GLFW_KEY_LEFT_SHIFT }, //TODO right shift
        { NK_KEY_CTRL, GLFW_KEY_LEFT_CONTROL }, //TODO right control
        //TODO all keys
        // NK_KEY_DEL,
        // NK_KEY_ENTER,
        // NK_KEY_TAB,
        // NK_KEY_BACKSPACE,
        // NK_KEY_UP,
        // NK_KEY_DOWN,
        // NK_KEY_LEFT,
        // NK_KEY_RIGHT,
    };
    size_t checked_keys_len = sizeof(checked_keys) / sizeof(checked_keys[0]);

    nk_input_begin(&m_ctx);

        //Mouse
        {
            //TODO check the int cast

            int mouse_x = static_cast<int>(mouse_pos.x), mouse_y = static_cast<int>(mouse_pos.y);
            // printf("input mouse pos: %f|%f, nuklear mouse pos: %d|%d\n", mouse_pos.x, mouse_pos.y, mouse_x, mouse_y);
            // printf("left is down: %d\n", (int)mouse_left_is_down);

            nk_input_motion(&m_ctx, mouse_x, mouse_y);

            nk_input_button(&m_ctx, NK_BUTTON_LEFT, mouse_x, mouse_y, (nk_bool)mouse_left_is_down); //TODO wrong x, y

            //TODO mouse scrolls
        }

        //Keys
        for (size_t i = 0; i < checked_keys_len; ++i)
        {
            nk_bool is_down = (glfwGetKey(window, checked_keys[i].second) == GLFW_PRESS);
            nk_input_key(&m_ctx, checked_keys[i].first, is_down);
        }

        //Input text
        for (size_t i = 0; i < textbuffer_len; ++i)
        {
            unsigned int codepoint = textbuffer[i];
            nk_input_unicode(&m_ctx, static_cast<nk_rune>(codepoint));
        }

    nk_input_end(&m_ctx);

    return true;
}

bool UI::Context::convert()
{
    assert(m_ctx_initialized); //DEBUG
    if (!m_ctx_initialized) return false;
    
    //TODO
    // result is one of:
    // * NK_CONVERT_SUCCESS              | Signals a successful draw command to vertex buffer conversion
    // * NK_CONVERT_INVALID_PARAM        | An invalid argument was passed in the function call
    // * NK_CONVERT_COMMAND_BUFFER_FULL  | The provided buffer for storing draw commands is full or failed to allocate more memory
    // * NK_CONVERT_VERTEX_BUFFER_FULL   | The provided buffer for storing vertices is full or failed to allocate more memory
    // * NK_CONVERT_ELEMENT_BUFFER_FULL  | The provided buffer for storing indices is full or failed to allocate more memory
    nk_flags result = nk_convert(&m_ctx, &m_cmd_buffer, &m_vert_buffer, &m_idx_buffer, &m_cfg);

    if (result != NK_CONVERT_SUCCESS)
    {
        fprintf(stderr, "[WARNING] Conversion of UI failed! Error value: '%d'!\n", result);
        return false;
    }

    return true;
}

bool UI::Context::draw(glm::vec2 screen_res, unsigned int texture_unit) //TODO read screen_res from some manager
{
    assert(m_ctx_initialized); //DEBUG
    if (!m_ctx_initialized) return false;

    // fills m_cmd_buffer, m_vert_buffer, m_idx_buffer with new data
    if (!convert()) return false;

    m_shader.use();
    glActiveTexture(GL_TEXTURE0 + texture_unit);
    
    //bind the respective OpenGL buffers
    assert(m_vbo_id != 0); //TODO empty id
    assert(m_ebo_id != 0); //TODO empty id
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo_id);

    //copy the (converted) data from Nuklear buffers into OpenGL buffers
    // printf("cmd allocated: %d, total: %d\n", m_cmd_buffer.allocated, nk_buffer_total(&m_cmd_buffer));
    // printf("vert allocated: %d, total: %d\n", m_vert_buffer.allocated, nk_buffer_total(&m_vert_buffer));
    // printf("idx allocated: %d, total: %d\n", m_idx_buffer.allocated, nk_buffer_total(&m_idx_buffer));
    glBufferData(GL_ARRAY_BUFFER, m_vert_buffer.allocated, nk_buffer_memory(&m_vert_buffer), GL_STREAM_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_idx_buffer.allocated, nk_buffer_memory(&m_idx_buffer), GL_STREAM_DRAW);
    // glBufferData(GL_ARRAY_BUFFER, nk_buffer_total(&m_vert_buffer), nk_buffer_memory(&m_vert_buffer), GL_STREAM_DRAW);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, nk_buffer_total(&m_idx_buffer), nk_buffer_memory(&m_idx_buffer), GL_STREAM_DRAW);

    //bind the screen resolution uniform
    m_shader.set("screenRes", screen_res);

    //setup the vbo attributes (buffer is already bound)
    {
        size_t stride = sizeof(UI::Vertex); //TODO alignment (just in case, seems like it's not needed with current UI::Vertex format)

        //position
        size_t pos_components = 2;
        size_t pos_offset = NK_OFFSETOF(UI::Vertex, pos); // offset is in bytes
        Shaders::setupVertexAttribute_float(Shaders::attribute_position_verts, pos_components,
                                            pos_offset, stride, true); // true specified for offset in bytes

        //texcoords
        size_t texcoords_components = 2;
        size_t texcoords_offset = NK_OFFSETOF(UI::Vertex, uv); // offset is in bytes
        Shaders::setupVertexAttribute_float(Shaders::attribute_position_texcoords, texcoords_components,
                                            texcoords_offset, stride, true); // true specified for offset in bytes

        //color
        size_t color_components = 4;
        size_t color_offset = NK_OFFSETOF(UI::Vertex, color); // offset is in bytes
        Shaders::setupVertexAttribute_ubyte(Shaders::attribute_position_color, color_components,
                                            color_offset, stride, true); // true specified for offset in bytes
    }

    size_t offset = 0;//, idx = 0;
    const struct nk_draw_command *cmd = NULL;
    nk_draw_foreach(cmd, &m_ctx, &m_cmd_buffer)
    {
        unsigned int elem_count = cmd->elem_count;
        if (!elem_count) {
            // ++idx;
            continue;
        };

        struct nk_rect clip_rect = cmd->clip_rect;
        int texture_id = cmd->texture.id;

        glBindTexture(GL_TEXTURE_2D, texture_id);
        // we need to mirror the scissor area because OpenGL window coordinates starts at bottom left
        // and nuclear ones at the top left
        glScissor(static_cast<GLint>(clip_rect.x),
                  //NOTE: this double cast is the only way it would work, not really sure why
                  static_cast<GLint>(screen_res.y - static_cast<GLint>(clip_rect.y + clip_rect.h)),
                  static_cast<GLint>(clip_rect.w),
                  static_cast<GLint>(clip_rect.h));
        // draw the ui element
        glDrawElements(GL_TRIANGLES, elem_count, GL_UNSIGNED_SHORT, reinterpret_cast<void*>(offset));

        //NOTE: this sizeof needs to be in line with type enum passed to glDrawElements (GL_UNSIGNED_SHORT)
        offset += elem_count * sizeof(GLushort);
    }

    //disable the vbo attributes
    Shaders::disableVertexAttribute(Shaders::attribute_position_verts);
    Shaders::disableVertexAttribute(Shaders::attribute_position_texcoords);
    Shaders::disableVertexAttribute(Shaders::attribute_position_color);

    //TODO unbinding in OpenGL is an anti-pattern
    //unbind the OpenGL buffers just to be sure
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return true;
}

void UI::Context::clear()
{
    if (!m_ctx_initialized) return;

    nk_buffer_clear(&m_cmd_buffer);
    nk_buffer_clear(&m_vert_buffer);
    nk_buffer_clear(&m_idx_buffer);

    nk_clear(&m_ctx);
}
