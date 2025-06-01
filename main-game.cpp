#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "game.hpp"
#include "stb_image.h"

#include "glm/gtc/matrix_transform.hpp"
#include <cstring>
#include <algorithm>


bool GameMainLoop::left_mbutton_state = false;

//TODO global manager for mouse callbacks! Call from different init will remove this one!
void GameMainLoop::mouseButtonsCallback(GLFWwindow *window, int button, int action, int mods)
{
    //TODO use glfwGetWindowUserPointer or make a manager for this callback

    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        GameMainLoop::left_mbutton_state = (action == GLFW_PRESS);
    }
}

//!IMPORTANT: we need to call destructor on objects even if their constructor "failed" (aka id == empty_id)
void GameMainLoop::initCamera()
{
    const glm::vec2 win_size = WindowManager::getSizeF();

    mouse_sens = 0.08f;
    fov = 80.f;
    camera_aspect_ratio = win_size.x / win_size.y;
    const glm::vec3 camera_init_pos(0.f, 0.7f, 2.5f);
    //const glm::vec3 camera_init_target = camera_init_pos + glm::vec3(0.f, 0.f, -1.f);
    //camera(fov, (float)window_width / (float)window_height, camera_init_pos, camera_init_target);
    camera_pitch = 0.f;
    camera_yaw = -90.f;
    new (&camera) Drawing::Camera3D(fov, camera_aspect_ratio, camera_init_pos, camera_pitch, camera_yaw);
}

void GameMainLoop::deinitCamera()
{
    camera.~Camera3D();
}

bool GameMainLoop::initVBOsAndMeshes()
{
    //Cube and it's vbo
    float cube_vertices[] = // counter-clockwise vertex winding order
    {
        //pos                //texcoords    //normals
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,   0.0f, 0.0f, -1.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,    0.0f, 0.0f, -1.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 0.0f,    0.0f, 0.0f, -1.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,    0.0f, 0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,   0.0f, 0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,   0.0f, 0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,   0.0f, 0.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,    0.0f, 0.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,    0.0f, 0.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,    0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,   0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,   0.0f, 0.0f, 1.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,   -1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,   -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,   -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,   -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,   -1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,   -1.0f, 0.0f, 0.0f,

        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,    1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  0.0f, 1.0f,    1.0f, 0.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,    1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  0.0f, 1.0f,    1.0f, 0.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,    1.0f, 0.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  0.0f, 0.0f,    1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,   0.0f, -1.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 1.0f,    0.0f, -1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,    0.0f, -1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,    0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,   0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,   0.0f, -1.0f, 0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,   0.0f, 1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,    0.0f, 1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,    0.0f, 1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,    0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,   0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,   0.0f, 1.0f, 0.0f,
    };
    size_t cube_vert_attrib = 8; //amount of cube attributes - 3x pos + 2x texcoords + 3x normals
    size_t cube_vert_count = (sizeof(cube_vertices) / sizeof(cube_vertices[0]))
                                / cube_vert_attrib; //amount of vertices - elements in array divided by attribute size

    new (&cube_vbo) Meshes::VBO(cube_vertices, cube_vert_count);
    if (cube_vbo.m_id == empty_id)
    {
        fprintf(stderr, "Failed to create cube VBO!\n");
        cube_vbo.~VBO();
        return false;
    }

    //Unit line and it's vbo
    float line_vertices[] =
    {
        //position
        0.f, 0.f,
        1.f, 1.f,
    };
    size_t line_vert_attrib = 2; //amount of line attributes - 2x pos
    size_t line_vert_count = (sizeof(line_vertices) / sizeof(line_vertices[0]))
                                / line_vert_attrib; //amount of vertices - elements in array divided by attribute size
    
    new (&line_vbo) Meshes::VBO(line_vertices, line_vert_count, Meshes::VBO::default2DConfig);
    if (line_vbo.m_id == empty_id)
    {
        fprintf(stderr, "Failed to create unit line VBO!\n");
        cube_vbo.~VBO();
        line_vbo.~VBO();
        return false;
    }

    //Turret mesh
    const char *turret_mesh_path = "assets/turret/turret.obj";

    new (&turret_mesh) Meshes::Mesh();
    int turret_mesh_ret = turret_mesh.loadFromObj(turret_mesh_path);
    if (turret_mesh_ret != 0 || !turret_mesh.isUploaded())
    {
        fprintf(stderr, "Failed to create Turret mesh! Returned error value: %d\n", turret_mesh_ret);
        cube_vbo.~VBO();
        line_vbo.~VBO();
        turret_mesh.~Mesh();
        return false;
    }

    //Ball mesh
    const char *ball_mesh_path = "assets/ball/dirty_football.obj";

    new (&ball_mesh) Meshes::Mesh();
    int ball_mesh_ret = ball_mesh.loadFromObj(ball_mesh_path);
    if (ball_mesh_ret != 0 || !ball_mesh.isUploaded())
    {
        fprintf(stderr, "Failed to create Bord Alternative mesh! Returned error value: %d\n", ball_mesh_ret);
        cube_vbo.~VBO();
        line_vbo.~VBO();
        turret_mesh.~Mesh();
        ball_mesh.~Mesh();
        return false;
    }

    return true;
}

void GameMainLoop::deinitVBOsAndMeshes()
{
    cube_vbo.~VBO();
    line_vbo.~VBO();
    turret_mesh.~Mesh();
    ball_mesh.~Mesh();
}

bool GameMainLoop::initTextures()
{
    using Texture = Textures::Texture2D;

    //Bricks
    const char *bricks_path = "assets/bricks2_512.png";
    brick_texture_world_size = glm::vec2(0.75f, 0.75f); // aspect ratio 1:1

    new (&brick_texture) Texture(bricks_path);
    if (brick_texture.m_id == empty_id)
    {
        fprintf(stderr, "Failed to create brick texture!\n");
        brick_texture.~Texture2D();
        return false;
    }

    const char *bricks_alt_path = "assets/bricks1.jpg";
    brick_alt_texture_world_size = glm::vec2(0.75f, 0.75f); // aspect ratio 1:1

    new (&brick_alt_texture) Texture(bricks_alt_path);
    if (brick_alt_texture.m_id == empty_id)
    {
        fprintf(stderr, "Failed to create brick alternative texture!\n");
        brick_texture.~Texture2D();
        brick_alt_texture.~Texture2D();
        return false;
    }

    //Orb
    const char *orb_path = "assets/orb_512.png";
    orb_texture_world_size = glm::vec2(1.f, 1.f); // almost 1:1 aspect ratio

    new (&orb_texture) Texture(orb_path);
    if (orb_texture.m_id == empty_id)
    {
        fprintf(stderr, "Failed to create orb texture!\n");
        brick_texture.~Texture2D();
        brick_alt_texture.~Texture2D();
        orb_texture.~Texture2D();
        return false;
    }

    //Target
    const char *target_path = "assets/target_256.png";

    new (&target_texture) Texture(target_path);
    if (target_texture.m_id == empty_id)
    {
        fprintf(stderr, "Failed to create target texture!\n");
        brick_texture.~Texture2D();
        brick_alt_texture.~Texture2D();
        orb_texture.~Texture2D();
        target_texture.~Texture2D();
        return false;
    }

    //Turret
    const char *turret_path = "assets/turret/turret_diffuse.png";

    new (&turret_texture) Texture(turret_path);
    if (turret_texture.m_id == empty_id)
    {
        fprintf(stderr, "Failed to create turret texture!\n");
        brick_texture.~Texture2D();
        brick_alt_texture.~Texture2D();
        orb_texture.~Texture2D();
        target_texture.~Texture2D();
        turret_texture.~Texture2D();
        return false;
    }

    //Ball
    //NOTE 4k textures might be problem in WebGL
    // const char *ball_tex_path = "assets/ball/textures/dirty_football_diff_4k.jpg";
    const char *ball_tex_path = "assets/ball/textures/dirty_football_diff_512.png";

    new (&ball_texture) Texture(ball_tex_path);
    if (ball_texture.m_id == empty_id)
    {
        fprintf(stderr, "Failed to create ball texture!\n");
        brick_texture.~Texture2D();
        brick_alt_texture.~Texture2D();
        orb_texture.~Texture2D();
        target_texture.~Texture2D();
        turret_texture.~Texture2D();
        ball_texture.~Texture2D();
        return false;
    }

    //Water specular map
    const char *water_specular_map_path = "assets/water_specular_map.jpg";

    new (&water_specular_map) Texture(water_specular_map_path);
    if (water_specular_map.m_id == empty_id)
    {
        fprintf(stderr, "Failed to create water specular map!\n");
        brick_texture.~Texture2D();
        brick_alt_texture.~Texture2D();
        orb_texture.~Texture2D();
        target_texture.~Texture2D();
        turret_texture.~Texture2D();
        ball_texture.~Texture2D();
        water_specular_map.~Texture2D();
        return false;
    }

    return true;
}

void GameMainLoop::deinitTextures()
{
    brick_texture.~Texture2D();
    brick_alt_texture.~Texture2D();
    orb_texture.~Texture2D();
    target_texture.~Texture2D();
    turret_texture.~Texture2D();
    ball_texture.~Texture2D();
    water_specular_map.~Texture2D();
}

/*bool GameMainLoop::initRenderBuffers()
{
    //TODO render buffer object abstraction
    fbo3d_rbo_depth = 0;
    fbo3d_rbo_stencil = 0;
    {
        assert(!Utils::checkForGLError());

        GLuint rbos[2];
        glGenRenderbuffers(2, rbos);
        if (Utils::checkForGLError())
        {
            fprintf(stderr, "Failed to create RenderBuffers for 3D FrameBuffer!\n");
            return false;
        }

        fbo3d_rbo_depth = rbos[0];
        fbo3d_rbo_stencil = rbos[1];
    }

    //depth renderbuffer allocation
    glBindRenderbuffer(GL_RENDERBUFFER, fbo3d_rbo_depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, fbo3d_tex.m_width, fbo3d_tex.m_height);

    //stencil renderbuffer allocation
    //TODO allow OpenGL 3.3 on dekstops to make stencil buffers work even on nvidia cards
    // glBindRenderbuffer(GL_RENDERBUFFER, fbo3d_rbo_stencil);
    // glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, fbo3d_tex.m_width, fbo3d_tex.m_height);

    glBindRenderbuffer(GL_RENDERBUFFER, empty_id);

    return true;
}

void GameMainLoop::deinitRenderBuffers()
{
    glDeleteBuffers(1, &fbo3d_rbo_depth);
    glDeleteBuffers(1, &fbo3d_rbo_stencil);
}*/

bool GameMainLoop::initShaders()
{
    //shader partials
    // used only during this init method, thus loaded as local unique_ptr, so we dont have to call delete/destructor
    const char *postprocess_fs_partial_path = SHADERS_PARTIALS_DIR_PATH "postprocess.fspart";
    std::unique_ptr<char[]> postprocess_fs_partial = Utils::getTextFileAsString(postprocess_fs_partial_path, NULL); //TODO check for NULL
    if (!postprocess_fs_partial)
    {
        fprintf(stderr, "Failed to load postprocessing fragment shader partial file: '%s'!\n", postprocess_fs_partial_path);
        return false;
    }

    using ShaderP = Shaders::Program;

    const char *default_vs_path = SHADERS_DIR_PATH "default.vs",
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
    std::vector<Shaders::ShaderInclude> ui_vs_includes = {},
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
    std::vector<Shaders::ShaderInclude> tex_rect_vs_includes{},
                                        tex_rect_fs_includes = {
                                                                // Shaders::ShaderInclude(postprocess_fs_partial.get()),
                                                               };

    new (&tex_rect_shader) ShaderP(transform_vs_path, tex_rect_fs_path, tex_rect_vs_includes, tex_rect_fs_includes);
    if (tex_rect_shader.m_id == empty_id)
    {
        fprintf(stderr, "Failed to create textured rectangle shader program!\n");
        screen_line_shader.~Program();
        ui_shader.~Program();
        tex_rect_shader.~Program();
        return false;
    }

    // loading special light source shader program (renders light source without light effects etc.)
    const char *light_src_fs_path = SHADERS_DIR_PATH "light_src.fs";
    
    new (&light_src_shader) ShaderP(default_vs_path, light_src_fs_path); // using the default vertex shader
    if (light_src_shader.m_id == empty_id)
    {
        fprintf(stderr, "Failed to create light source shader program!\n");
        screen_line_shader.~Program();
        ui_shader.~Program();
        tex_rect_shader.~Program();
        light_src_shader.~Program();
        return false;
    }

    // loading light shader program
    const char *light_vs_path = SHADERS_DIR_PATH "texture.vs"; // using the texture vertex shader (maybe change this?)
    const char *light_fs_path = SHADERS_DIR_PATH "light.fs";
    std::vector<Shaders::ShaderInclude> light_vs_includes = {},
                                        light_fs_includes =
                                            {
                                                // TODO synchronize this value with `lights_max_amount` constexpr
                                                Shaders::ShaderInclude(Shaders::IncludeDefine("LIGHTS_MAX_AMOUNT", "10")),
                                                Shaders::ShaderInclude(Shaders::IncludeDefine("ALPHA_MIN_THRESHOLD", "0.35")),
                                            };

    new (&light_shader) ShaderP(light_vs_path, light_fs_path, light_vs_includes, light_fs_includes);
    if (light_shader.m_id == empty_id)
    {
        fprintf(stderr, "Failed to create shader program for lighting!\n");
        screen_line_shader.~Program();
        ui_shader.~Program();
        tex_rect_shader.~Program();
        light_src_shader.~Program();
        light_shader.~Program();
        return false;
    }

    return true;
}

void GameMainLoop::deinitShaders()
{
    screen_line_shader.~Program();
    ui_shader.~Program();
    tex_rect_shader.~Program();
    light_src_shader.~Program();
    light_shader.~Program();
}

void GameMainLoop::initLighting()
{
    using LightProps = Lighting::LightProps;
    using DirLight = Lighting::DirLight;
    // using PointLight = Lighting::PointLight;
    using SpotLight = Lighting::SpotLight;
    
    light_src_size = 0.2;

    //sun
    const LightProps sun_light_props(Color3F(0.9f, 0.8f, 0.7f), 0.4f);
    new (&sun) DirLight(sun_light_props, glm::vec3(1.f, -1.f, -0.5f));

    //flashlight
    const Color3F flashlight_color = Color3F(1.f, 1.f, 1.f);
    const LightProps flashlight_light_props(flashlight_color, 0.f);

    new (&flashlight) SpotLight(flashlight_light_props, glm::vec3(0.f), glm::vec3(0.f), //throwaway values for position and direction
                                40.f, 50.f);
    flashlight.setAttenuation(1.f, 0.09f, 0.032f);
    show_flashlight = false;
}

void GameMainLoop::deinitLighting()
{
    sun.~DirLight();
    flashlight.~SpotLight();
}

void GameMainLoop::initMaterials()
{
    const SharedGLContext& shared_gl_context = SharedGLContext::instance.value();
    assert(shared_gl_context.isInitialized());
    const Textures::Texture2D& white_pixel = shared_gl_context.white_pixel_tex;

    using MaterialProps = Lighting::MaterialProps;
    using Material = Lighting::Material;

    //Default props
    new (&default_material_props) MaterialProps(Color3F(1.0f, 1.0f, 1.0f), 32.f);

    //Ball props
    // const char *ball_mtl_path = "assets/ball/dirty_football.mtl";

    //TODO proper materials - https://www.fileformat.info/format/material/
    // std::vector<MaterialProps> ball_materials{};
    // if (Meshes::loadMtl(ball_mtl_path, ball_materials) || ball_materials.size() == 0)
    // {
    //     fprintf(stderr, "[WARNING] Failed to load material for ball mesh, default material will be used.\n");
    //     ball_material_props = default_material;
    // }
    // else
    // {
    //     if (ball_materials_props.size() > 1) fprintf(stderr, "[WARNING] Multiple materials loaded for ball mesh, using the first one.\n");
    //     ball_material_props = ball_materials[0];
    // }
    // printf("ball material - ambient: %f|%f|%f, diffuse: %f|%f|%f, specular: %f|%f|%f, shinines: %f\n",
    //        ball_material.m_ambient.r, ball_material.m_ambient.g, ball_material.m_ambient.b,
    //        ball_material.m_diffuse.r, ball_material.m_diffuse.g, ball_material.m_diffuse.b,
    //        ball_material.m_specular.r, ball_material.m_specular.g, ball_material.m_specular.b, ball_material.m_shininess);

    // ball_material_props = default_material_props;
    const MaterialProps ball_material_props = MaterialProps{Color3F{1.f}, Color3F{1.f}, Color3F{0.2f}, 1.f};

    //Default material
    new (&default_material) Material(default_material_props, white_pixel, white_pixel);

    //Ball material
    new (&ball_material) Material(ball_material_props, ball_texture, white_pixel);

    //Target material
    const MaterialProps target_material_props{Color3F{1.f}, Color3F{1.f}, Color3F{0.f}, 1.f};
    new (&target_material) Material(target_material_props, target_texture, white_pixel);
}

void GameMainLoop::deinitMaterials()
{
    default_material_props.~MaterialProps(); //TODO useless now as there is no destructor yet

    default_material.~Material();
    ball_material.~Material();
    target_material.~Material();
}

bool GameMainLoop::initUI()
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

void GameMainLoop::deinitUI()
{
    font.~Font();
    ui.~Context();
}

/*bool GameMainLoop::initFrameBuffers()
{
    using FrameBuffer = Drawing::FrameBuffer;
    
    new (&fbo3d) FrameBuffer(true);
    if (fbo3d.m_id == empty_id)
    {
        fprintf(stderr, "Failed to initialize FrameBuffer for 3D scene!\n");
        fbo3d.~FrameBuffer();
        return false;
    }

    fbo3d.attachAll(fbo3d_tex.asFrameBufferAttachment(),
                    FrameBuffer::Attachment{ fbo3d_rbo_depth, FrameBuffer::AttachmentType::render },
                    // FrameBuffer::Attachment{ fbo3d_rbo_stencil, FrameBuffer::AttachmentType::render }
                    FrameBuffer::Attachment{ 0, FrameBuffer::AttachmentType::none } //TODO fix stencil buffer not working on nvidia
                    );
    if (!fbo3d.isComplete())
    {
        fprintf(stderr, "FrameBuffer for 3D scene is not complete!\n");
        fbo3d.~FrameBuffer();
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, empty_id);

    return true;
}

void GameMainLoop::deinitFrameBuffers()
{
    fbo3d.~FrameBuffer();
}*/

bool GameMainLoop::initGameStuff()
{
    //Wall and it's vbo
    wall_size = glm::vec3(5.f, 2.5f, 0.2f);
    wall_pos = glm::vec3(0.f, wall_size.y / 2.f, 0.f);
    
    // rewrite the potential garbage values to 0 and id to empty_id
    // memset is here just in case, but the id might be needed for following move assign operator
    memset((void*)&wall_vbo, 0, sizeof(wall_vbo));
    wall_vbo.m_id = empty_id;
    wall_vbo = std::move(Meshes::generateCubicVBO(wall_size, brick_alt_texture_world_size,
                                                  Meshes::TexcoordStyle::repeat, true));
    if (wall_vbo.m_id == empty_id)
    {
        fprintf(stderr, "Failed to create wall VBO!\n");
        wall_vbo.~VBO();
        return false;
    }

    //Targets
    using Target = Game::Target;

    new (&target_mesh) Meshes::Mesh();
    target_mesh = std::move(Meshes::generateQuadMesh(glm::vec2(1.f), target_texture_world_size, Meshes::TexcoordStyle::stretch));
    if (!target_mesh.isUploaded())
    {
        fprintf(stderr, "Failed to create target's Mesh!\n");
        wall_vbo.~VBO();
        target_mesh.~Mesh();
        return false;
    }

    new (&target_model) Meshes::Model(light_shader, target_mesh, target_material);
    assert(target_texture_world_size.x == target_texture_world_size.y);
    const float target_dish_world_size = target_texture_world_size.x * target_texture_dish_radius * 2.f;
    target_model.m_scale *= Game::Target::flat_target_size / target_dish_world_size;

    new (&ball_model) Meshes::Model(light_shader, ball_mesh, ball_material);
    ball_model.m_origin_offset = ball_origin_offset;
    const float ball_world_size = ball_world_radius * 2.f;
    ball_model.m_scale *= Game::Target::ball_target_size / ball_world_size;
    // const Color3F ball_model_color_tint(0.8f, 0.3f, 0.15f);
    const Color3F ball_model_color_tint(1.f, 1.f, 1.f);
    ball_model.m_material.m_props.m_ambient = ball_model_color_tint;
    ball_model.m_material.m_props.m_diffuse = ball_model_color_tint;

    wall_center = glm::vec3(0.f, wall_size.y / 2.f, wall_size.z / 2.f);
    new (&targets) std::vector<Target>();
    new (&ball_targets) std::vector<Target>();

    //Targets rng init
    new (&target_rng_width) Utils::RNG(-1000, 1000);
    new (&target_rng_height) Utils::RNG(-500, 500);

    //Level manager
    using Level = Game::Level;
    using LevelPart = Game::LevelPart;
    using TargetType = Game::TargetType;

    new (&level_manager) Game::LevelManager();

    level_manager.addLevel(Level{ std::vector<LevelPart>{ LevelPart{ TargetType::target, 1, 0.5f,
                                                                     Game::targetMiddleWallPosition, std::monostate{}, NULL,
                                                                     Color3F(0.3f, 0.9f, 0.6f) } }, true });
    level_manager.addLevel(Level{ std::vector<LevelPart>{ LevelPart{ TargetType::target, 3, 0.6f } } });
    level_manager.addLevel(Level{ std::vector<LevelPart>{ LevelPart{ TargetType::target, 5, 0.65f } } });
    level_manager.addLevel(Level{ std::vector<LevelPart>{ LevelPart{ TargetType::ball, 6, 0.7f,
                                                                     Game::targetRandomWallPosition,
                                                                    //  std::monostate{},
                                                                     Game::PosChanger_float::Params{ glm::vec2(1.f), glm::vec3(0.f), 2.5f },
                                                                     Game::targetGetScale_linearFactor<3>,
                                                                     Color3F(0.8f, 0.3f, 0.15f) },
                                                          LevelPart{ TargetType::target, 6, 4.f,
                                                                     Game::targetRandomWallPosition, std::monostate{}, NULL,
                                                                     Color3F(0.35f, 0.6f, 0.9f) } } });
    level_manager.addLevel(Level{ std::vector<LevelPart>{ LevelPart{ TargetType::target, 15, 0.85f } }, true });

    const glm::vec2 wall_size_quarter2d = glm::vec2{ wall_size.x, wall_size.y } / 4.f;
    const glm::vec3 wall_size_center_UL{ -wall_size_quarter2d.x, wall_size_quarter2d.y, 0.f };
    const glm::vec3 wall_size_center_UR{ wall_size_quarter2d.x, wall_size_quarter2d.y, 0.f };
    const glm::vec3 wall_size_center_DR{ wall_size_quarter2d.x, -wall_size_quarter2d.y, 0.f };
    const glm::vec3 wall_size_center_DL{ -wall_size_quarter2d.x, -wall_size_quarter2d.y, 0.f };
    level_manager.addLevel(Level{ std::vector<LevelPart>{ LevelPart{ TargetType::ball, 4, 1.2f,
                                                                     Game::targetMiddleWallPosition,
                                                                     Game::PosChanger_float::Params{ glm::vec2(0.5f), wall_size_center_UL, 2.f } } }, true });
    level_manager.addLevel(Level{ std::vector<LevelPart>{ LevelPart{ TargetType::ball, 4, 1.2f,
                                                                     Game::targetMiddleWallPosition,
                                                                     Game::PosChanger_float::Params{ glm::vec2(0.5f), wall_size_center_UR, 2.f } } }, true });
    level_manager.addLevel(Level{ std::vector<LevelPart>{ LevelPart{ TargetType::ball, 4, 1.2f,
                                                                     Game::targetMiddleWallPosition,
                                                                     Game::PosChanger_float::Params{ glm::vec2(0.5f), wall_size_center_DR, 2.f } } }, true });
    level_manager.addLevel(Level{ std::vector<LevelPart>{ LevelPart{ TargetType::ball, 4, 1.2f,
                                                                     Game::targetMiddleWallPosition,
                                                                     Game::PosChanger_float::Params{ glm::vec2(0.5f), wall_size_center_DL, 2.f } } }, true });
    
    level_manager.addLevel(Level{ std::vector<LevelPart>{ LevelPart{ TargetType::target, 30, 1.3f } }, true });

    //Target practice stuff
    practice_time_start = -1.f;
    practice_time_end = -1.f;
    new (&pracice_times) std::vector<float>();

    return true;
}

void GameMainLoop::deinitGameStuff()
{
    wall_vbo.~VBO();
    target_mesh.~Mesh();
    target_model.~Model();
    ball_model.~Model();
    targets.~vector();
    ball_targets.~vector();
    target_rng_width.~RNG();
    target_rng_height.~RNG();
    level_manager.~LevelManager();
    pracice_times.~vector();
}

unsigned int GameMainLoop::getTargetsAlive() const
{
    return static_cast<unsigned int>(targets.size() + ball_targets.size());
}

void GameMainLoop::handleTargetHit(double current_frame_time)
{
    bool next_level_reached = level_manager.handleTargetHit(current_frame_time);

    if (next_level_reached && level_manager.m_level_idx == 1) // first level started
    {
        practice_time_start = current_frame_time;
        practice_time_end = -1.f;
    }

    if (level_manager.levelsCompleted()) // all levels completed
    {
        level_manager.prepareFirstLevel(current_frame_time);
        practice_time_end = current_frame_time;

        //save the time
        assert(practice_time_start >= 0.f);
        float practice_time = static_cast<float>(practice_time_end - practice_time_start);
        assert(practice_time >= 0.f);
        pracice_times.push_back(practice_time);
        printf("Practice time: %.3fs\n", practice_time);
    }
}

int GameMainLoop::init()
{
    puts("GameMainLoop init begin");

    GLFWwindow * const window = WindowManager::getWindow();

    //mouse callback setup
    glfwSetMouseButtonCallback(window, &GameMainLoop::mouseButtonsCallback); //TODO mouse manager

    //Camera
    initCamera();

    //VBOs
    if (!initVBOsAndMeshes())
    {
        //TODO goto cleanup routine?
        deinitCamera();
        return 1;
    }

    //Textures
    if (!initTextures())
    {
        //TODO goto cleanup routine?
        deinitCamera();
        deinitVBOsAndMeshes();
        return 2;
    }

    //RenderBuffers
    // if (!initRenderBuffers())
    // {
    //     //TODO goto cleanup routine?
    //     deinitCamera();
    //     deinitVBOsAndMeshes();
    //     deinitTextures();
    //     return 3;
    // }

    //Shaders
    if (!initShaders())
    {
        //TODO goto cleanup routine?
        deinitCamera();
        deinitVBOsAndMeshes();
        deinitTextures();
        // deinitRenderBuffers();
        return 4;
    }

    //Lighting
    initLighting();

    //Materials
    initMaterials();

    //UI
    if (!initUI())
    {
        //TODO goto cleanup routine?
        deinitCamera();
        deinitVBOsAndMeshes();
        deinitTextures();
        // deinitRenderBuffers();
        deinitShaders();
        deinitLighting();
        deinitMaterials();
        return 5;
    }

    //Framebuffers
    // if (!initFrameBuffers())
    // {
    //     //TODO goto cleanup routine?
    //     deinitCamera();
    //     deinitVBOsAndMeshes();
    //     deinitTextures();
    //     // deinitRenderBuffers();
    //     deinitShaders();
    //     deinitLighting();
    //     deinitMaterials();
    //     deinitUI();
    //     return 6;
    // }

    //Game stuff
    if (!initGameStuff())
    {
        //TODO goto cleanup routine?
        deinitCamera();
        deinitVBOsAndMeshes();
        deinitTextures();
        // deinitRenderBuffers();
        deinitShaders();
        deinitLighting();
        deinitMaterials();
        deinitUI();
        // deinitFrameBuffers();
        return 7;
    }

    //Misc.
    clear_color_3d = Color(50, 220, 80);
    // clear_color_3d = Color(10, 10, 10); //DEBUG
    clear_color_2d = Color(0, 0, 0);
    tick = 0;
    frame_delta = 0.f;
    last_frame_time = glfwGetTime();
    fps_calculation_interval = 0.5f; // in seconds
    last_fps_calculation_time = last_frame_time;
    fps_calculation_counter = 0;
    fps_calculated = 0;
    last_mouse_x = 0.f;
    last_mouse_y = 0.f;
    last_left_mbutton = false;

    puts("GameMainLoop init end");
    return 0;
}

GameMainLoop::~GameMainLoop()
{
    // glDeleteRenderbuffers(1, &fbo3d_rbo_depth);
    // glDeleteRenderbuffers(1, &fbo3d_rbo_stencil);
}

LoopRetVal GameMainLoop::loop()
{
    GLFWwindow * const window = WindowManager::getWindow();
    const glm::vec2 win_size = WindowManager::getSizeF();
    SharedGLContext& shared_gl_context = SharedGLContext::instance.value();
    assert(shared_gl_context.isInitialized());

    //calculating correct frame delta time
    //TODO this will be wrong when stacking of game loops gets implemented
    double current_frame_time = glfwGetTime();
    frame_delta = current_frame_time - last_frame_time;
    last_frame_time = current_frame_time;

    glfwPollEvents();

    // ---Mouse input---
    double mouse_x = 0.f, mouse_y = 0.f;
    glfwGetCursorPos(window, &mouse_x, &mouse_y);

    if (tick == 0)
    {
        last_mouse_x = mouse_x;
        last_mouse_y = mouse_y;
    }

    float mouse_delta_x = (float)(mouse_x - last_mouse_x), mouse_delta_y = (float)(mouse_y - last_mouse_y);
    //printf("mouse_delta_x: %f, mouse_delta_y: %f\n", mouse_delta_x, mouse_delta_y);

    if (!CLOSE_TO_0(mouse_delta_x) || !CLOSE_TO_0(mouse_delta_y))
    {
        camera_yaw += mouse_sens * mouse_delta_x;
        camera_pitch = std::min(89.f, std::max(-89.f, camera_pitch - mouse_sens * mouse_delta_y)); // set camera_pitch with limits -89°/89°
        camera.setTargetFromPitchYaw(camera_pitch, camera_yaw); //TODO make this better
    }

    //updating camera's aspect ratio if the window aspect ration changed
    const float win_aspect_ratio = win_size.x / win_size.y;
    if (!FLOAT_EQUALS(camera_aspect_ratio, win_aspect_ratio))
    {
        camera_aspect_ratio = win_aspect_ratio;
        camera.setProjectionMatrix(fov, camera_aspect_ratio);
    }

    const glm::vec2 mouse_pos(static_cast<float>(mouse_x), static_cast<float>(mouse_y));
    const Collision::Ray mouse_ray = camera.getRay();

    // ---Keyboard input---
    bool mbutton_left_is_pressed = left_mbutton_state;
    bool mbutton_left_is_clicked = left_mbutton_state && !last_left_mbutton;

    if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

    if(glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) show_flashlight = true;
    if(glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) show_flashlight = false;

    glm::vec3 move_dir_rel = Movement::getSimplePlayerDir(window);

    // ---Shooting---
    if (tick == 0)
    {
        level_manager.prepareFirstLevel(current_frame_time);
    }

    //TODO this system does not work properly when shooting from an angle - ball gets hit even when aiming at the flat target
    // possible fix could be to also check for a hit of the wall, if target hit is further from the player than the wall hit count it as a miss
    if (mbutton_left_is_clicked)
    {
        size_t hit_idx = 0;

        //ball targets - they go first as they get hit before flat targets
        Collision::RayCollision rcoll = Collision::rayBallTargets(mouse_ray, ball_targets, current_frame_time, &hit_idx);
        if (rcoll.m_hit)
        {
            assert(hit_idx < ball_targets.size());
            ball_targets.erase(ball_targets.begin() + hit_idx); // delete the target at `out_idx`
            handleTargetHit(current_frame_time);
        }
        else
        {
            //flat targets - only if no ball target hit
            rcoll = Collision::rayFlatTargets(mouse_ray, targets, current_frame_time, &hit_idx);
            if (rcoll.m_hit)
            {
                assert(hit_idx < targets.size());
                targets.erase(targets.begin() + hit_idx); // delete the target at `out_idx`
                handleTargetHit(current_frame_time);
            }
        }
    }

    // ---Target spawning---
    {
        // not using radius as we would *2 for both borders
        glm::vec2 flat_target_spawn_area = glm::vec2(wall_size.x, wall_size.y)
                                            - glm::vec2(Game::Target::flat_target_size);
        glm::vec2 ball_target_spawn_area = glm::vec2(wall_size.x, wall_size.y)
                                            - glm::vec2(Game::Target::ball_target_size);
        
        unsigned int targets_alive = getTargetsAlive();
        unsigned int target_spawn_amount = level_manager.targetSpawnAmount(current_frame_time, targets_alive);

        for (unsigned int i = 0; i < target_spawn_amount; ++i)
        {
            const Game::LevelPart *current_level_part = level_manager.getCurrentLevelPart(targets_alive + i); // +i as we spawned i targets already
            assert(current_level_part != NULL);
            if (current_level_part)
            {
                Game::Target::ScaleFnPtr *scale_fn = current_level_part->m_scale_fn;
                Color3F color = current_level_part->m_color;

                switch(current_level_part->m_type)
                {
                case Game::TargetType::target:
                    {
                        targets.emplace_back(target_model, current_frame_time,
                                             current_level_part->spawnNext(target_rng_width, target_rng_height, wall_center, flat_target_spawn_area),
                                             color, scale_fn);
                        break;
                    }
                case Game::TargetType::ball:
                    {
                        ball_targets.emplace_back(ball_model, current_frame_time,
                                                  current_level_part->spawnNext(target_rng_width, target_rng_height, wall_center, ball_target_spawn_area),
                                                  color, scale_fn);
                        break;
                    }
                default: assert(false); // unimplemented case for TargetType enum
                }
            }
        }
    }

    // ---Target position updating---
    for (size_t i = 0; i < targets.size(); ++i)
    {
        targets[i].updatePos(current_frame_time);
    }
    for (size_t i = 0; i < ball_targets.size(); ++i)
    {
        ball_targets[i].updatePos(current_frame_time);
    }

    // ---Player movement---
    const float move_per_sec = 4.f;
    const float move_magnitude = move_per_sec * frame_delta;
    glm::vec3 move_dir_abs = camera.dirCoordsViewToWorld(move_dir_rel); // absolute move dir vector (world coords)
    glm::vec3 move_abs = move_magnitude * NORMALIZE_OR_0(glm::vec3(move_dir_abs.x, 0.f, move_dir_abs.z)); // we remove the vertical movement
    // glm::vec3 move_abs = move_magnitude * move_dir_abs;
    
    if (!Utils::isZero(move_abs))
    {
        // printf("move_rel: %f|%f|%f\n", move_rel.x, move_rel.y, move_rel.z);
        // printf("move_abs: %f|%f|%f\n", move_abs.x, move_abs.y, move_abs.z);
        camera.move(move_abs);
    }

    // ---Lights---
    //lights array
    std::vector<std::reference_wrapper<const Lighting::Light>> lights = { sun };

    //flashlight
    if (show_flashlight)
    {
        // updating the flashlight position and directiom
        flashlight.m_pos = camera.m_pos;
        flashlight.m_dir = camera.getDirection();

        lights.push_back(flashlight);
    }

    // ---UI---
    //pump the input into UI
    if (!ui.getInput(window, mouse_pos, mbutton_left_is_pressed, textbuffer, textbuffer_len))
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
        
        //General info
        if (nk_begin(&ui.m_ctx, "Target Practice", nk_rect(30, 30, 150, 240),
            NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_NO_SCROLLBAR))
        {
            unsigned int level = level_manager.m_level_idx,
                         level_targets_hit = level_manager.m_level_targets_hit,
                         level_amount = level_manager.getLevelAmount(),
                         level_target_amount = level_manager.getCurrentLevelTargetAmount(),
                         whole_target_amount = level_manager.getWholeTargetAmount();

            nk_layout_row_dynamic(&ui.m_ctx, 0, 1);

            //fps calculating and rendering
            ++fps_calculation_counter;
            if (current_frame_time >= last_fps_calculation_time + fps_calculation_interval)
            {
                const double time_passed = current_frame_time - last_fps_calculation_time;
                fps_calculated = static_cast<unsigned int>(static_cast<double>(fps_calculation_counter) / time_passed);
                
                fps_calculation_counter = 0;
                last_fps_calculation_time = current_frame_time;
            }
            snprintf(ui_textbuff, ui_textbuff_capacity, "FPS: %d", fps_calculated);
            nk_label(&ui.m_ctx, ui_textbuff, NK_TEXT_LEFT);

            //level counter
            nk_layout_row_begin(&ui.m_ctx, NK_DYNAMIC, 20, 2);
            {
                nk_layout_row_push(&ui.m_ctx, 0.5f);
                nk_label(&ui.m_ctx, "Level:", NK_TEXT_LEFT);

                nk_layout_row_push(&ui.m_ctx, 0.5f);
                snprintf(ui_textbuff, ui_textbuff_capacity, "%d/%d",
                         level, level_amount ? level_amount - 1 : 0); // -1 as we dont count the intro level
                nk_label(&ui.m_ctx, ui_textbuff, NK_TEXT_RIGHT);
            }
            nk_layout_row_end(&ui.m_ctx);

            //target counter
            nk_layout_row_begin(&ui.m_ctx, NK_DYNAMIC, 20, 2);
            {
                nk_layout_row_push(&ui.m_ctx, 0.5f);
                nk_label(&ui.m_ctx, "Progress:", NK_TEXT_LEFT);

                nk_layout_row_push(&ui.m_ctx, 0.5f);
                snprintf(ui_textbuff, ui_textbuff_capacity, "%d/%d", level_targets_hit, level_target_amount);
                nk_label(&ui.m_ctx, ui_textbuff, NK_TEXT_RIGHT);
            }
            nk_layout_row_end(&ui.m_ctx);

            ui.horizontalGap(8);

            //level progress bar
            {
                ui.m_ctx.style.progress.cursor_normal = nk_style_item_color(nk_rgb(100, 100, 100));
            }
            nk_layout_row_dynamic(&ui.m_ctx, 30, 1);
            {
                nk_size level_targets_hit_copy = level_targets_hit;
                nk_progress(&ui.m_ctx, &level_targets_hit_copy, level_target_amount, NK_FIXED); // ignoring the return value as we use NK_FIXED
            }

            //complete progress bar
            {
                ui.m_ctx.style.progress.cursor_normal = nk_style_item_color(nk_rgb(180, 180, 180));
            }
            nk_layout_row_dynamic(&ui.m_ctx, 30, 1);
            {
                nk_size whole_targets_hit_copy = level_manager.getPartialTargetAmount(0, level_manager.m_level_idx) + level_targets_hit;
                nk_progress(&ui.m_ctx, &whole_targets_hit_copy, whole_target_amount, NK_FIXED); // ignoring the return value as we use NK_FIXED
            }

            //practice time
            nk_layout_row_begin(&ui.m_ctx, NK_DYNAMIC, 20, 2);
            {
                double time;
                if (practice_time_start < 0.f) time = 0.f;
                else if (practice_time_end < 0.f) time = current_frame_time - practice_time_start;
                else time = practice_time_end - practice_time_start;

                nk_layout_row_push(&ui.m_ctx, 0.5f);
                nk_label(&ui.m_ctx, "Time:", NK_TEXT_LEFT);

                nk_layout_row_push(&ui.m_ctx, 0.5f);
                snprintf(ui_textbuff, ui_textbuff_capacity, "%.3fs", time);
                nk_label(&ui.m_ctx, ui_textbuff, NK_TEXT_RIGHT);
            }
            nk_layout_row_end(&ui.m_ctx);
        }
        nk_end(&ui.m_ctx);

        //Time window
        const int time_window_header_height = 50, time_window_height_min = time_window_header_height + 40;
        int time_window_height = std::max<int>(time_window_height_min,
                                               time_window_header_height + 25 * pracice_times.size());
        if (nk_begin(&ui.m_ctx, "Previous times:", nk_rect(30, 280, 150, time_window_height),
            NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_NO_SCROLLBAR))
        {
            nk_layout_row_dynamic(&ui.m_ctx, 20, 1);

            for (size_t i = 0; i < pracice_times.size(); ++i)
            {
                snprintf(ui_textbuff, ui_textbuff_capacity, "%.3fs", pracice_times[i]);
                nk_label(&ui.m_ctx, ui_textbuff, NK_TEXT_RIGHT);
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
            ui.horizontalGap(25.f);
        #endif
        nk_label(&ui.m_ctx, "Ondrej Richtr, 2025", NK_TEXT_RIGHT);
    }
    nk_end(&ui.m_ctx);

    // ---Drawing---
    {
        bool use_fbo = shared_gl_context.use_fbo3d;

        //3D block
        {
            const Drawing::FrameBuffer& fbo3d = shared_gl_context.getFbo3D();

            //set the viewport according to wanted framebuffer
            if (use_fbo)
            {
                const glm::ivec2 fbo3d_size = shared_gl_context.getFbo3DSize();
                glViewport(0, 0, fbo3d_size.x, fbo3d_size.y);
            }
            else
            {
                const glm::ivec2 fbo3d_win_size = WindowManager::getFBOSize();
                glViewport(0, 0, fbo3d_win_size.x, fbo3d_win_size.y);
            }
            
            //bind the correct framebuffer
            if (use_fbo) fbo3d.bind();
            else glBindFramebuffer(GL_FRAMEBUFFER, empty_id);

            Drawing::clear(clear_color_3d);
            glDepthMask(GL_TRUE); // must enable depth buffer, so the clear will work properly
            glStencilMask(0xFF);  // must enable stencil buffer, so the clear will work properly
            glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); //TODO make this nicer - probably move into Drawing

            const glm::mat4& view_mat = camera.getViewMatrix();
            const glm::mat4& proj_mat = camera.getProjectionMatrix();

            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);

            glDisable(GL_STENCIL_TEST);
            glStencilMask(0x00);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

            //Enable backface culling
            glCullFace(GL_BACK);
            glEnable(GL_CULL_FACE);

            //cube
            light_shader.use();
            {
                glm::vec3 pos = glm::vec3(-4.f, 0.35f, -0.5f);

                //vs
                glm::mat4 model_mat(1.f);
                model_mat = glm::translate(model_mat, pos);
                model_mat = glm::scale(model_mat, glm::vec3(0.7f, 0.7f, 0.7f));

                glm::mat3 normal_mat = Utils::modelMatrixToNormalMatrix(model_mat);

                light_shader.set("model", model_mat);
                light_shader.set("normalMat", normal_mat);
                light_shader.set("view", view_mat);
                light_shader.set("projection", proj_mat);

                //fs
                light_shader.set("cameraPos", camera.m_pos);
                light_shader.setMaterialProps(default_material_props);
                light_shader.bindDiffuseMap(brick_texture);
                light_shader.bindSpecularMap(shared_gl_context.white_pixel_tex);
                light_shader.setLights(UNIFORM_LIGHT_NAME, UNIFORM_LIGHT_COUNT_NAME, lights); // return value ignored here
            }

            cube_vbo.bind();
                glDrawArrays(GL_TRIANGLES, 0, cube_vbo.vertexCount());
            cube_vbo.unbind();

            //turret
            light_shader.use();
            {
                glm::vec3 pos = glm::vec3(4.3f, 0.f, -1.5f);
                glm::vec3 scale = glm::vec3(0.3f);

                //vs
                glm::mat4 model_mat(1.f);
                model_mat = glm::translate(model_mat, pos);
                model_mat = glm::scale(model_mat, scale);
                model_mat = glm::rotate(model_mat, glm::pi<float>(), Drawing::up_dir); // rotate towards the player spawn point

                glm::mat3 normal_mat = Utils::modelMatrixToNormalMatrix(model_mat);

                light_shader.set("model", model_mat);
                light_shader.set("normalMat", normal_mat);
                light_shader.set("view", view_mat);
                light_shader.set("projection", proj_mat);

                //fs
                light_shader.set("cameraPos", camera.m_pos);
                light_shader.setMaterialProps(default_material_props);
                light_shader.bindDiffuseMap(turret_texture);
                light_shader.bindSpecularMap(shared_gl_context.white_pixel_tex);
                light_shader.setLights(UNIFORM_LIGHT_NAME, UNIFORM_LIGHT_COUNT_NAME, lights); // return value ignored here
            }
            
            turret_mesh.draw();

            //ball
            light_shader.use();
            {
                glm::vec3 pos = glm::vec3(2.2f, 0.f, 2.2f);
                glm::vec3 scale = glm::vec3(3.f);

                //vs
                glm::mat4 model_mat(1.f);
                model_mat = glm::translate(model_mat, pos);
                model_mat = glm::scale(model_mat, scale);

                glm::mat3 normal_mat = Utils::modelMatrixToNormalMatrix(model_mat);

                light_shader.set("model", model_mat);
                light_shader.set("normalMat", normal_mat);
                light_shader.set("view", view_mat);
                light_shader.set("projection", proj_mat);

                //fs
                light_shader.set("cameraPos", camera.m_pos);
                light_shader.setMaterial(ball_material);
                light_shader.setLights(UNIFORM_LIGHT_NAME, UNIFORM_LIGHT_COUNT_NAME, lights); // return value ignored here
            }
            
            ball_mesh.draw();

            //ball with an outline
            glEnable(GL_STENCIL_TEST);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);  
            glStencilFunc(GL_ALWAYS, 1, 0xFF); // all fragments should pass the stencil test
            glStencilMask(0xFF); // enable writing to the stencil buffer if it wasn't already
            {
                glm::vec3 pos = glm::vec3(3.6f, 0.33f, 2.2f);
                glm::vec3 scale = glm::vec3(2.5f);
                const float outline_scale_factor = 1.1f;
                const Color3F outline_color(0.f, 1.f, 1.f);

                //drawing the object itself
                light_shader.use();
                {
                    //vs
                    glm::mat4 model_mat(1.f);
                    model_mat = glm::translate(model_mat, pos);
                    model_mat = glm::scale(model_mat, scale);
                    model_mat = glm::translate(model_mat, ball_origin_offset);

                    glm::mat3 normal_mat = Utils::modelMatrixToNormalMatrix(model_mat);

                    light_shader.set("model", model_mat);
                    light_shader.set("normalMat", normal_mat);
                    light_shader.set("view", view_mat);
                    light_shader.set("projection", proj_mat);

                    //fs
                    light_shader.set("cameraPos", camera.m_pos);
                    light_shader.setMaterial(ball_material);
                    light_shader.setLights(UNIFORM_LIGHT_NAME, UNIFORM_LIGHT_COUNT_NAME, lights); // return value ignored here
                }
                ball_mesh.draw();

                //drawing the outline - attempt it only with OpenGL 3.3 or WebGL
                //TODO delete this condition after implementing stencil buffer everywhere
                #if defined(BUILD_OPENGL_330_CORE) || defined(PLATFORM_WEB)
                    light_src_shader.use();
                    ball_texture.bind();
                    {
                        //vs
                        glm::mat4 model_mat(1.f);
                        model_mat = glm::translate(model_mat, pos);
                        model_mat = glm::scale(model_mat, scale * outline_scale_factor);
                        model_mat = glm::translate(model_mat, ball_origin_offset);

                        light_src_shader.set("model", model_mat);
                        light_src_shader.set("view", view_mat);
                        light_src_shader.set("projection", proj_mat);

                        //fs
                        light_src_shader.set("lightSrcColor", outline_color);
                    }

                    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
                    // glDisable(GL_DEPTH_TEST); //TODO this also?
                    glStencilMask(0x00); // disable writing to the stencil buffer
                    ball_mesh.draw();
                #endif
            }
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);
            glDepthFunc(GL_LESS);
            glDisable(GL_STENCIL_TEST);
            glStencilMask(0x00);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

            //ball with default material
            light_shader.use();
            {
                glm::vec3 pos = glm::vec3(-2.2f, -0.5f, 2.2f);
                glm::vec3 scale = glm::vec3(3.f);

                //vs
                glm::mat4 model_mat(1.f);
                model_mat = glm::translate(model_mat, pos);
                model_mat = glm::scale(model_mat, scale);

                glm::mat3 normal_mat = Utils::modelMatrixToNormalMatrix(model_mat);

                light_shader.set("model", model_mat);
                light_shader.set("normalMat", normal_mat);
                light_shader.set("view", view_mat);
                light_shader.set("projection", proj_mat);

                //fs
                light_shader.set("cameraPos", camera.m_pos);
                light_shader.setMaterialProps(default_material_props);
                light_shader.bindDiffuseMap(ball_texture);
                light_shader.bindSpecularMap(shared_gl_context.white_pixel_tex);
                light_shader.setLights(UNIFORM_LIGHT_NAME, UNIFORM_LIGHT_COUNT_NAME, lights); // return value ignored here
            }

            ball_mesh.draw();

            //wall
            light_shader.use();
            {
                //vs
                glm::mat4 model_mat(1.f);
                model_mat = glm::translate(model_mat, wall_pos);

                glm::mat3 normal_mat = Utils::modelMatrixToNormalMatrix(model_mat);

                light_shader.set("model", model_mat);
                light_shader.set("normalMat", normal_mat);
                light_shader.set("view", view_mat);
                light_shader.set("projection", proj_mat);

                //fs
                light_shader.set("cameraPos", camera.m_pos);
                light_shader.setMaterialProps(default_material_props);
                light_shader.bindDiffuseMap(brick_alt_texture);
                light_shader.bindSpecularMap(shared_gl_context.white_pixel_tex);
                light_shader.setLights(UNIFORM_LIGHT_NAME, UNIFORM_LIGHT_COUNT_NAME, lights); // return value ignored here
            }

            wall_vbo.bind();
                glDrawArrays(GL_TRIANGLES, 0, wall_vbo.vertexCount());
            wall_vbo.unbind();

            //targets - must be rendered after the wall they are attachedd to!
            // we also disable culling and depth buffer writing (so the z-fighting does not happen)
            glDisable(GL_CULL_FACE);
            glDepthMask(GL_FALSE);
            glDepthFunc(GL_LEQUAL);

            const size_t tagets_amount = targets.size();
            for (size_t i = 0; i < tagets_amount; ++i)
            {
                const glm::vec3 pos_offset = glm::vec3(0.f, 0.f, FLOAT_TOLERANCE);
                targets[i].draw(Game::TargetType::target, camera, lights, current_frame_time, pos_offset);
            }

            //ball targets
            // we change the GL settings back
            glEnable(GL_CULL_FACE);
            glDepthMask(GL_TRUE);
            glDepthFunc(GL_LESS);

            const size_t ball_targets_amount = ball_targets.size();
            for (size_t i = 0; i < ball_targets_amount; ++i)
            {
                ball_targets[i].draw(Game::TargetType::ball, camera, lights, current_frame_time);
            }

            if (use_fbo) fbo3d.unbind();

            assert(!Utils::checkForGLErrorsAndPrintThem()); //DEBUG
        }

        //2D block
        {
            //TODO use correct win size + check whether some functions need it as parameter
            const glm::vec2 win_fbo_size = WindowManager::getFBOSizeF();
            const glm::ivec2 win_fbo_size_i = WindowManager::getFBOSize();
            glm::vec2 window_middle = win_fbo_size / 2.f;

            //TODO this might be wrong on some displays?
            //set the viewport according to window size
            glViewport(0, 0, win_fbo_size_i.x, win_fbo_size_i.y);

            //bind the default framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, empty_id);
            if (use_fbo)
            {
                Drawing::clear(clear_color_2d);
                //TODO maybe useless in 2D block?
                glClear(GL_DEPTH_BUFFER_BIT); //TODO make this nicer - probably move into Drawing
            }

            glDepthMask(GL_FALSE);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glEnable(GL_BLEND); //TODO check this
            glBlendEquation(GL_FUNC_ADD); //TODO check this
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //TODO check this
            
            //render the 3D scene as a background from it's framebuffer
            if (use_fbo)
            {
                const Textures::Texture2D& fbo3d_tex = shared_gl_context.getFbo3DTexture();
                // Drawing::texturedRectangle2(tex_rect_shader, fbo3d_tex, orb_texture, brick_texture, glm::vec2(0.f), win_fbo_size);
                Drawing::texturedRectangle(tex_rect_shader, fbo3d_tex, win_fbo_size, glm::vec2(0.f), win_fbo_size);
            }
            
            //line test
            // Drawing::screenLine(screen_line_shader, line_vbo, win_size,
            //                     screen_middle, glm::vec2(50.f),
            //                     50.f, ColorF(1.0f, 0.0f, 0.0f));

            //crosshair
            const ColorF crosshair_color = ColorF(1.f, 1.f, mbutton_left_is_pressed ? 1.f : 0.f);
            Drawing::crosshair(screen_line_shader, line_vbo, win_fbo_size,
                                glm::vec2(50.f, 30.f), window_middle, 1.f, crosshair_color);

            //UI drawing
            glEnable(GL_SCISSOR_TEST); // enable scissor for UI drawing only
            if (!ui.draw(win_fbo_size))
            {
                fprintf(stderr, "[WARNING] Failed to draw the UI!\n");
            }
            glDisable(GL_SCISSOR_TEST);

            glDisable(GL_BLEND);

            assert(!Utils::checkForGLErrorsAndPrintThem()); //DEBUG
        }
    }
    
    ui.clear(); // UI clear is here as we want to call it each frame regardless of drawing stage

    last_mouse_x = mouse_x;
    last_mouse_y = mouse_y;
    last_left_mbutton = left_mbutton_state;
    ++tick;

    return LoopRetVal::success;
}
