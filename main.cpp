#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "game.hpp"
#include "stb_image.h"

#include "glm/gtc/matrix_transform.hpp" //DEBUG

#include <algorithm>


//TODO move this elsewhere - window manager?
int window_width = DEFAULT_WINDOW_WIDTH, window_height = DEFAULT_WINDOW_HEIGHT;

void windowResizeCallback(GLFWwindow* window, int width, int height)
{
    //TODO check for resizes to 0x0?
    window_width = width;
    window_height = height;

    //TODO use glfwGetWindowUserPointer

    printf("Window resized to: %dx%d\n", window_width, window_height);
}

//TODO this callback seems pretty pointless since glfwPollEvents should cache the state by itself?
bool left_mbutton_state = false; // true means pressed, false means released

void mouseButtonsCallback(GLFWwindow *window, int button, int action, int mods)
{
    //TODO use glfwGetWindowUserPointer

    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        // left_mbutton_prev_state = left_mbutton_state;

        left_mbutton_state = (action == GLFW_PRESS);
    }
}

int test_main(void)
{
    puts("Program begin.");

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
    GLFWwindow* window = glfwCreateWindow(window_width, window_height,
                                          (char*)window_title, NULL, NULL);
    if (window == NULL)
    {
        fprintf(stderr, "GLFW failed to create window!\n");
        glfwTerminate();
        return 2;
    }
    glfwMakeContextCurrent(window);

    //other GLFW settings
    glfwSwapInterval(1); //TODO check this, maybe 0?
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetFramebufferSizeCallback(window, windowResizeCallback);
    //glfwSetCursorPosCallback(window, windowMouseMoveCallback); 

    //initializing GLAD
    if (!gladLoadGLES2Loader((GLADloadproc)glfwGetProcAddress))
    {
        fprintf(stderr, "GLAD failed to initialize!\n");
        glfwTerminate();
        return 3;
    }

    windowResizeCallback(window, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);

    //Camera
    using Camera = Drawing::Camera3D;

    const float mouse_sens = 0.08f;
    float fov = 80.f;
    const glm::vec3 camera_init_pos(0.f, 0.f, 2.5f);
    //const glm::vec3 camera_init_target = camera_init_pos + glm::vec3(0.f, 0.f, -1.f);
    //Camera camera(fov, (float)window_width / (float)window_height, camera_init_pos, camera_init_target);
    float camera_pitch = 0.f;
    float camera_yaw = -90.f;
    Camera camera(fov, (float)window_width / (float)window_height, camera_init_pos, camera_pitch, camera_yaw);

    //Triangle and it's vbo
    GLfloat trinagle_verts[] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.0f,  0.5f, 0.0f
    };
    GLfloat triangle_tex_verts[] = {
        0.0f, 0.0f,  
        1.0f, 0.0f,
        0.5f, 1.0f
    };

    unsigned int triangle_vbo; //TODO make this better
    glGenBuffers(1, &triangle_vbo);
    // unsigned int vao;
    // glGenVertexArrays(1, &vao);

    // glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, triangle_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(trinagle_verts), trinagle_verts, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // glVertexAttribPointer(0, 3, GL_FLOAT, false, 3 * sizeof(GLfloat), (void*)0);
    // glEnableVertexAttribArray(0);

    //Square and it's vbo and ebo
    GLfloat square_vertices[] = {
        //positions             //texcoords     //normals
        0.2f,  0.2f,   0.0f,     1.0f, 1.0f,    0.0f, 0.0f, 1.0f, // top right front
        0.2f,  -0.2f,  0.0f,     1.0f, 0.0f,    0.0f, 0.0f, 1.0f, // bottom right front
        -0.2f,  -0.2f, 0.0f,     0.0f, 0.0f,    0.0f, 0.0f, 1.0f, // bottom left front
        -0.2f,  0.2f,  0.0f,     0.0f, 1.0f,    0.0f, 0.0f, 1.0f, // top left  front
        
        0.2f,  0.2f,   0.0f,     1.0f, 1.0f,    0.0f, 0.0f, -1.0f, // top right back
        0.2f,  -0.2f,  0.0f,     1.0f, 0.0f,    0.0f, 0.0f, -1.0f, // bottom right back
        -0.2f,  -0.2f, 0.0f,     0.0f, 0.0f,    0.0f, 0.0f, -1.0f, // bottom left back
        -0.2f,  0.2f,  0.0f,     0.0f, 1.0f,    0.0f, 0.0f, -1.0f  // top left  back
    };
    GLuint square_indices[] = {
        0, 1, 3,   // first front triangle
        1, 2, 3,   // second front triangle

        4, 5, 7,   // first back triangle
        5, 6, 7    // second back triangle
    };
    size_t square_verts_pos_offset = 0;
    size_t square_verts_texcoord_offset = 3;
    size_t square_verts_normal_offset = 5;
    size_t square_vert_attrib = 8; //amount of square attributes - 3x pos + 2x texcoords + 3x normals
    size_t square_poly_count = sizeof(square_indices) / sizeof(square_indices[0]);

    unsigned int square_vbo; //TODO make this better
    glGenBuffers(1, &square_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, square_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(square_vertices), square_vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    unsigned int square_ebo;
    glGenBuffers(1, &square_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, square_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(square_indices), square_indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    //Cube and it's vbo
    float cube_vertices[] = {
        //pos                //texcoords    //normals
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,   0.0f, 0.0f, -1.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 0.0f,    0.0f, 0.0f, -1.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,    0.0f, 0.0f, -1.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,    0.0f, 0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,   0.0f, 0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,   0.0f, 0.0f, -1.0f,

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
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,    1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  0.0f, 1.0f,    1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  0.0f, 1.0f,    1.0f, 0.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  0.0f, 0.0f,    1.0f, 0.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,    1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,   0.0f, -1.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 1.0f,    0.0f, -1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,    0.0f, -1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,    0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,   0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,   0.0f, -1.0f, 0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,   0.0f, 1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,    0.0f, 1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,    0.0f, 1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,    0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,   0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,   0.0f, 1.0f, 0.0f,
    };
    size_t cube_verts_pos_offset = 0;
    size_t cube_verts_texcoord_offset = 3;
    size_t cube_verts_normal_offset = 5;
    size_t cube_vert_attrib = 8; //amount of cube attributes - 3x pos + 2x texcoords + 3x normals
    size_t cube_vert_count = (sizeof(cube_vertices) / sizeof(cube_vertices[0]))
                                / cube_vert_attrib; //amount of vertices - elements in array divided by attribute size

    unsigned int cube_vbo; //TODO make this better
    glGenBuffers(1, &cube_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //Default shader program
    using ShaderP = Shaders::Program;

    const char *default_vs_path = "shaders/default.vs",
               *default_fs_path = "shaders/default.fs";
    
    ShaderP default_shader(default_vs_path, default_fs_path);
    if (default_shader.m_id == Shaders::empty_id)
    {
        fprintf(stderr, "Failed to create default shader program!\n");
        glfwTerminate();
        return 4;
    }

    //Textures
    // loading simple texture shader program
    const char *texture_vs_path = "shaders/texture.vs",
               *texture_fs_path = "shaders/texture.fs";
    
    ShaderP texture_shader(texture_vs_path, texture_fs_path);
    if (texture_shader.m_id == Shaders::empty_id)
    {
        fprintf(stderr, "Failed to create simple texture shader program!\n");
        glfwTerminate();
        return 5;
    }

    using Texture = Textures::Texture2D;

    const char *bricks_path = "assets/bricks2.png",
               *orb_path    = "assets/orb.jpg";

    Texture brick_texture(bricks_path);
    if (brick_texture.m_id == Textures::empty_id)
    {
        fprintf(stderr, "Failed to create brick texture!\n");
        glfwTerminate();
        return 6;
    }
    //TODO rework this
    texture_shader.use();
    texture_shader.set("inputTexture1", 0);

    Texture orb_texture(orb_path);
    if (orb_texture.m_id == Textures::empty_id)
    {
        fprintf(stderr, "Failed to create orb texture!\n");
        glfwTerminate();
        return 7;
    }
    //TODO rework this
    texture_shader.use();
    texture_shader.set("inputTexture2", 1);

    //Light sources
    // loading special light source shader program (renders light source without light effects etc.)
    const char *light_src_fs_path = "shaders/light_src.fs";
    
    ShaderP light_src_shader(default_vs_path, light_src_fs_path); // using the default vertex shader
    if (light_src_shader.m_id == Shaders::empty_id)
    {
        fprintf(stderr, "Failed to create light source shader program!\n");
        glfwTerminate();
        return 8;
    }

    // loading light shader program
    const char *light_fs_path = "shaders/light.fs";

    ShaderP light_shader(texture_vs_path, light_fs_path); // using the texture vertex shader (maybe change this?)
    if (light_shader.m_id == Shaders::empty_id)
    {
        fprintf(stderr, "Failed to create shader program for lighting!\n");
        glfwTerminate();
        return 9;
    }

    const float light_src_size = 0.2;
    using LightProps = Lighting::LightProps;
    using DirLight = Lighting::DirLight;
    using PointLight = Lighting::PointLight;
    using SpotLight = Lighting::SpotLight;

    //sun
    const LightProps sun_light_props(Color3F(0.9f, 0.8f, 0.7f), 0.4f);
    DirLight sun(sun_light_props, glm::vec3(1.f, -1.f, -0.5f));

    //point light test
    const Color3F pointl_color = Color3F(0.1f, 0.3f, 0.95f);
    const float pointl_ambient_intensity = 0.1f;
    const LightProps pointl_light_props(pointl_color, pointl_ambient_intensity);

    PointLight pointl(pointl_light_props, glm::vec3(-2.5f, -0.3f, 0.6f));
    pointl.setAttenuation(1.f, 0.09f, 0.032f);
    // pointl.setAttenuation(1.f, 0.22f, 0.2f);
    const Color3F pointl_spec_color = pointl.m_props.m_specular;

    //moving light test
    const Color3F movingl_color = Color3F(9.8f, 8.5f, 0.f);
    const LightProps movingl_light_props(movingl_color, 0.1f);
    glm::vec3 mat_cubes_pos(-8.f * 0.8f, -0.4f, 2.f);
    float movingl_x_min = -8.f * 0.8f;
    float movingl_x_max = -8.f * 0.8f + (16 - 1) * 0.8f;
    float movingl_move_per_sec = 2.f;

    PointLight movingl(movingl_light_props, glm::vec3(movingl_x_min, -0.4f, 0.9f));
    // SpotLight movingl(movingl_light_props, glm::vec3(0.f, 0.f, 1.f), glm::vec3(movingl_x_min, -0.4f, 0.9f),
    //                   12.5f, 17.5f);
    // movingl.setAttenuation(1.f, 0.09f, 0.032f);
    movingl.setAttenuation(1.f, 0.22f, 0.2f);
    bool movingl_pos_move = true;

    //flashlight
    const Color3F flashlight_color = Color3F(1.f, 1.f, 1.f);
    const LightProps flashlight_light_props(flashlight_color, 0.f);

    SpotLight flashlight(flashlight_light_props, glm::vec3(0.f), glm::vec3(0.f), //throwaway values for position and direction
                         40.f, 50.f);
    flashlight.setAttenuation(1.f, 0.09f, 0.032f);
    bool show_flashlight = false;

    //Materials
    using MaterialProps = Lighting::MaterialProps;

    MaterialProps default_material(Color3F(1.0f, 1.0f, 1.0f), 32.f);

    size_t materials_count = 16;
    MaterialProps materials[16];

    for (size_t i = 0, shininess = 1; i < materials_count; ++i)
    {
        materials[i] = MaterialProps(Color3F(0.65f, 0.4f, 0.5f), i ? 10 * i : 1);
        glm::vec3 spec = glm::vec3(1.f) * ((float)(i + 1) / (float)materials_count);
        materials[i].m_specular = Color3F(spec);
    }

    // MainLoopData loopData = {}; //TODO use this
    // loopData.test = 15;
    // loopData.window = window;

    //Misc.
    Color clear_color(50, 220, 80);
    unsigned int tick = 0;
    float frame_delta = 0.f;
    double last_frame_time = glfwGetTime();
    double last_mouse_x = 0.f, last_mouse_y = 0.f;

    //DEBUG
    bool show_pointl = true;
    
    while(!glfwWindowShouldClose(window))
    {
        //calculating correct frame delta time
        double current_frame_time = glfwGetTime();
        frame_delta = current_frame_time - last_frame_time;
        last_frame_time = current_frame_time;

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

        if (mouse_delta_x != 0.f || mouse_delta_y != 0.f)
        {
            camera_yaw += mouse_sens * mouse_delta_x;
            camera_pitch = std::min(89.f, std::max(-89.f, camera_pitch - mouse_sens * mouse_delta_y)); // set camera_pitch with limits -89°/89°
            camera.setTargetFromPitchYaw(camera_pitch, camera_yaw); //TODO make this better
        }

        // ---Keyboard input---
        if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

        if(glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) show_pointl = true;
        if(glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) show_pointl = false;

        if(glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) show_flashlight = true;
        if(glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) show_flashlight = false;

        if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        {
            camera_yaw -= 0.5f;
            camera.setTargetFromPitchYaw(camera_pitch, camera_yaw);
        }
        if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        {
            camera_yaw += 0.5f;
            camera.setTargetFromPitchYaw(camera_pitch, camera_yaw);
        }
        if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        {
            camera_pitch += 0.5f;
            camera.setTargetFromPitchYaw(camera_pitch, camera_yaw);
        }
        if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        {
            camera_pitch -= 0.5f;
            camera.setTargetFromPitchYaw(camera_pitch, camera_yaw);
        }

        glm::vec3 move_dir = Movement::getSimplePlayerDir(window);

        // ---Camera movement---
        const float move_per_sec = 4.f;
        const float move_magnitude = move_per_sec * frame_delta;
        glm::vec3 move_rel = move_magnitude * move_dir; // move vector relative to the camera (view coords)
        
        if (!Utils::isZero(move_rel))
        {
            glm::vec3 move_abs = camera.dirCoordsViewToWorld(move_rel); // absolute move vector (world coords)
            // printf("move_rel: %f|%f|%f\n", move_rel.x, move_rel.y, move_rel.z);
            // printf("move_abs: %f|%f|%f\n", move_abs.x, move_abs.y, move_abs.z);
            camera.move(move_abs);
        }

        //point light turning on/off
        if (show_pointl)
        {
            pointl.m_props = LightProps(pointl_color, pointl_ambient_intensity);
            pointl.m_props.m_specular = pointl_spec_color;
        }
        else
        {
            pointl.m_props = LightProps(Color3F(0.f, 0.f, 0.f), pointl_ambient_intensity);
            pointl.m_props.m_specular = glm::vec3(0.f);
        }

        // ---Light movement---
        float movingl_move_magnitude = movingl_move_per_sec * frame_delta;
        if (movingl_pos_move)
        {
            movingl.m_pos.x += movingl_move_magnitude;
            if (movingl.m_pos.x > movingl_x_max)
            {
                movingl.m_pos.x = movingl_x_max;
                movingl_pos_move = false;
            }
        }
        else
        {
            movingl.m_pos.x -= movingl_move_magnitude;
            if (movingl.m_pos.x < movingl_x_min)
            {
                movingl.m_pos.x = movingl_x_min;
                movingl_pos_move = true;
            }
        }

        //updating flashlight position and direction according to camera
        flashlight.m_pos = camera.m_pos;
        flashlight.m_dir = camera.getDirection();

        // ---Draw begin---
        {
            Drawing::clear(clear_color);
            glClear(GL_DEPTH_BUFFER_BIT); //TODO make this nicer - probably move into Drawing

            const glm::mat4& view_mat = camera.getViewMatrix();
            const glm::mat4& proj_mat = camera.getProjectionMatrix();

            //3D block
            {
                glEnable(GL_DEPTH_TEST);

                //triangle
                default_shader.use();
                {
                    float time = glfwGetTime();
                    float blue_value = sin(time) / 2.0f + 0.5f;
                    default_shader.set("inputColor", glm::vec4(0.0f, 0.0f, blue_value, 1.0f));

                    glm::mat4 model_mat(1.f);
                    model_mat = glm::translate(model_mat, glm::vec3(-0.7f, 0.3f, 0.0f));
                    //model_mat = glm::rotate(model_mat, time, glm::vec3(0.f, 1.f, 0.f));
                    model_mat = glm::rotate(model_mat, time, glm::vec3(0.f, 0.f, 1.f));

                    default_shader.set("model", model_mat);
                    default_shader.set("view", view_mat);
                    default_shader.set("projection", proj_mat);
                }
                
                glBindBuffer(GL_ARRAY_BUFFER, triangle_vbo);
                    Shaders::setupVertexAttribute_float(0, 3, 0, 3 * sizeof(GLfloat));
                        glDrawArrays(GL_TRIANGLES, 0, 3);
                    Shaders::disableVertexAttribute(0);
                glBindBuffer(GL_ARRAY_BUFFER, 0);

                //TODO culling
                // glCullFace(GL_BACK);
                // glEnable(GL_CULL_FACE);

                //square
                /*texture_shader.use();
                brick_texture.bind(0);
                orb_texture.bind(1);
                {
                    texture_shader.set("lightSrcColor", light_src.m_diffuse);
                    texture_shader.set("lightSrcPos", light_src.m_pos);
                    texture_shader.set("cameraPos", camera.m_pos);

                    glm::mat4 model_mat(1.f);
                    model_mat = glm::translate(model_mat, glm::vec3(0.7f, 0.7f, 0.f));

                    glm::mat3 normal_mat = Utils::modelMatrixToNormalMatrix(model_mat);

                    texture_shader.set("model", model_mat);
                    texture_shader.set("normalMat", normal_mat);
                    texture_shader.set("view", view_mat);
                    texture_shader.set("projection", proj_mat);
                }

                glBindBuffer(GL_ARRAY_BUFFER, square_vbo);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, square_ebo);
                    Shaders::setupVertexAttribute_float(0, 3, square_verts_pos_offset, square_vert_attrib * sizeof(GLfloat));
                    Shaders::setupVertexAttribute_float(1, 2, square_verts_texcoord_offset, square_vert_attrib * sizeof(GLfloat));
                    Shaders::setupVertexAttribute_float(2, 3, square_verts_normal_offset, square_vert_attrib * sizeof(GLfloat));
                        glDrawElements(GL_TRIANGLES, square_poly_count, GL_UNSIGNED_INT, NULL);
                    Shaders::disableVertexAttribute(0);
                    Shaders::disableVertexAttribute(1);
                    Shaders::disableVertexAttribute(2);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);*/

                //spinny cube
                /*texture_shader.use();
                brick_texture.bind(0);
                orb_texture.bind(1);
                {
                    texture_shader.set("lightSrcColor", light_src.m_diffuse);
                    texture_shader.set("lightSrcPos", light_src.m_pos);
                    texture_shader.set("cameraPos", camera.m_pos);

                    float time = glfwGetTime();
                    glm::mat4 model_mat(1.f);
                    model_mat = glm::scale(model_mat, glm::vec3(0.5f, 0.5f, 0.5f));
                    model_mat = glm::rotate(model_mat, time, glm::vec3(0.f, 1.f, 0.f));
                    model_mat = glm::rotate(model_mat, time, glm::vec3(0.f, 0.f, 1.f));

                    glm::mat3 normal_mat = Utils::modelMatrixToNormalMatrix(model_mat);

                    texture_shader.set("model", model_mat);
                    texture_shader.set("normalMat", normal_mat);
                    texture_shader.set("view", view_mat);
                    texture_shader.set("projection", proj_mat);
                }

                glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
                    Shaders::setupVertexAttribute_float(0, 3, cube_verts_pos_offset, cube_vert_attrib * sizeof(GLfloat));
                    Shaders::setupVertexAttribute_float(1, 2, cube_verts_texcoord_offset, cube_vert_attrib * sizeof(GLfloat));
                    Shaders::setupVertexAttribute_float(2, 3, cube_verts_normal_offset, cube_vert_attrib * sizeof(GLfloat));
                        glDrawArrays(GL_TRIANGLES, 0, cube_vert_count);
                    Shaders::disableVertexAttribute(0);
                    Shaders::disableVertexAttribute(1);
                    Shaders::disableVertexAttribute(2);
                glBindBuffer(GL_ARRAY_BUFFER, 0);*/

                //test point light source cube
                light_src_shader.use();
                {
                    light_src_shader.set("lightSrcColor", pointl.m_props.m_diffuse);

                    glm::mat4 model_mat(1.f);
                    model_mat = glm::translate(model_mat, pointl.m_pos);
                    model_mat = glm::scale(model_mat, glm::vec3(light_src_size));

                    light_src_shader.set("model", model_mat);
                    light_src_shader.set("view", view_mat);
                    light_src_shader.set("projection", proj_mat);
                }

                glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
                    Shaders::setupVertexAttribute_float(0, 3, cube_verts_pos_offset, cube_vert_attrib * sizeof(GLfloat));
                    // Shaders::setupVertexAttribute_float(1, 2, cube_verts_texcoord_offset, cube_vert_attrib * sizeof(GLfloat));
                    // Shaders::setupVertexAttribute_float(2, 3, cube_verts_normal_offset, cube_vert_attrib * sizeof(GLfloat));
                        glDrawArrays(GL_TRIANGLES, 0, cube_vert_count);
                    Shaders::disableVertexAttribute(0);
                    // Shaders::disableVertexAttribute(1);
                    // Shaders::disableVertexAttribute(2);
                glBindBuffer(GL_ARRAY_BUFFER, 0);

                //moving point light source cube
                light_src_shader.use();
                {
                    light_src_shader.set("lightSrcColor", movingl.m_props.m_diffuse);

                    glm::mat4 model_mat(1.f);
                    model_mat = glm::translate(model_mat, movingl.m_pos);
                    model_mat = glm::scale(model_mat, glm::vec3(light_src_size));

                    light_src_shader.set("model", model_mat);
                    light_src_shader.set("view", view_mat);
                    light_src_shader.set("projection", proj_mat);
                }

                glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
                    Shaders::setupVertexAttribute_float(0, 3, cube_verts_pos_offset, cube_vert_attrib * sizeof(GLfloat));
                    // Shaders::setupVertexAttribute_float(1, 2, cube_verts_texcoord_offset, cube_vert_attrib * sizeof(GLfloat));
                    // Shaders::setupVertexAttribute_float(2, 3, cube_verts_normal_offset, cube_vert_attrib * sizeof(GLfloat));
                        glDrawArrays(GL_TRIANGLES, 0, cube_vert_count);
                    Shaders::disableVertexAttribute(0);
                    // Shaders::disableVertexAttribute(1);
                    // Shaders::disableVertexAttribute(2);
                glBindBuffer(GL_ARRAY_BUFFER, 0);

                //material cubes
                light_shader.use();
                brick_texture.bind();
                {
                    //vs
                    light_shader.set("view", view_mat);
                    light_shader.set("projection", proj_mat);
                    
                    //fs
                    light_shader.set("cameraPos", camera.m_pos);
                    light_shader.setLight(UNIFORM_LIGHT_NAME, sun, 0);
                    light_shader.setLight(UNIFORM_LIGHT_NAME, pointl, 1);
                    light_shader.setLight(UNIFORM_LIGHT_NAME, movingl, 2);
                    if (show_flashlight) light_shader.setLight(UNIFORM_LIGHT_NAME, flashlight, 3);
                    light_shader.set(UNIFORM_LIGHT_COUNT_NAME, show_flashlight ? 4 : 3);
                }

                glm::vec3 mat_cubes_pos(-8.f * 0.8f, -0.4f, 2.f);

                for (size_t i = 0; i < materials_count; ++i)
                {
                    //vs
                    // float time = glfwGetTime();
                    glm::mat4 model_mat(1.f);
                    model_mat = glm::translate(model_mat, mat_cubes_pos);
                    model_mat = glm::scale(model_mat, glm::vec3(0.7f, 0.7f, 0.7f));
                    // model_mat = glm::rotate(model_mat, time, glm::vec3(0.f, 1.f, 0.f));
                    // model_mat = glm::rotate(model_mat, time, glm::vec3(0.f, 0.f, 1.f));

                    glm::mat3 normal_mat = Utils::modelMatrixToNormalMatrix(model_mat);

                    light_shader.set("model", model_mat);
                    light_shader.set("normalMat", normal_mat);

                    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
                        Shaders::setupVertexAttribute_float(0, 3, cube_verts_pos_offset, cube_vert_attrib * sizeof(GLfloat));
                        Shaders::setupVertexAttribute_float(1, 2, cube_verts_texcoord_offset, cube_vert_attrib * sizeof(GLfloat));
                        Shaders::setupVertexAttribute_float(2, 3, cube_verts_normal_offset, cube_vert_attrib * sizeof(GLfloat));
                            glDrawArrays(GL_TRIANGLES, 0, cube_vert_count);
                        Shaders::disableVertexAttribute(0);
                        Shaders::disableVertexAttribute(1);
                        Shaders::disableVertexAttribute(2);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);

                    mat_cubes_pos.x += 0.8f;

                    //fs
                    light_shader.setMaterialProps(materials[i]);
                }

                //default material cube
                light_shader.use();
                brick_texture.bind();
                {
                    //vs
                    glm::mat4 model_mat(1.f);
                    model_mat = glm::translate(model_mat, glm::vec3(-2.f, 0.f, -0.5f));
                    model_mat = glm::scale(model_mat, glm::vec3(0.7f, 0.7f, 0.7f));

                    glm::mat3 normal_mat = Utils::modelMatrixToNormalMatrix(model_mat);

                    light_shader.set("model", model_mat);
                    light_shader.set("normalMat", normal_mat);
                    light_shader.set("view", view_mat);
                    light_shader.set("projection", proj_mat);

                    //fs
                    light_shader.set("cameraPos", camera.m_pos);
                    light_shader.setMaterialProps(default_material);
                    light_shader.setLight(UNIFORM_LIGHT_NAME, sun, 0);
                    light_shader.setLight(UNIFORM_LIGHT_NAME, pointl, 1);
                    light_shader.setLight(UNIFORM_LIGHT_NAME, movingl, 2);
                    if (show_flashlight) light_shader.setLight(UNIFORM_LIGHT_NAME, flashlight, 3);
                    light_shader.set(UNIFORM_LIGHT_COUNT_NAME, show_flashlight ? 4 : 3);
                }

                glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
                    Shaders::setupVertexAttribute_float(0, 3, cube_verts_pos_offset, cube_vert_attrib * sizeof(GLfloat));
                    Shaders::setupVertexAttribute_float(1, 2, cube_verts_texcoord_offset, cube_vert_attrib * sizeof(GLfloat));
                    Shaders::setupVertexAttribute_float(2, 3, cube_verts_normal_offset, cube_vert_attrib * sizeof(GLfloat));
                        glDrawArrays(GL_TRIANGLES, 0, cube_vert_count);
                    Shaders::disableVertexAttribute(0);
                    Shaders::disableVertexAttribute(1);
                    Shaders::disableVertexAttribute(2);
                glBindBuffer(GL_ARRAY_BUFFER, 0);


                glDisable(GL_CULL_FACE);
                glDisable(GL_DEPTH_TEST);
            }
        }

        // ---Draw end---
        glfwPollEvents();
        glfwSwapBuffers(window);

        last_mouse_x = mouse_x;
        last_mouse_y = mouse_y;
        ++tick;
    }

    glfwTerminate();

    puts("Program end.");
    return 0;
}

int game_main(void)
{
    puts("Game begin.");

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
    GLFWwindow* window = glfwCreateWindow(window_width, window_height,
                                          (char*)window_title, NULL, NULL);
    if (window == NULL)
    {
        fprintf(stderr, "GLFW failed to create window!\n");
        glfwTerminate();
        return 2;
    }
    glfwMakeContextCurrent(window);

    //other GLFW settings
    glfwSwapInterval(1); //TODO check this, maybe 0?
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); //DEBUG
    glfwSetFramebufferSizeCallback(window, windowResizeCallback);
    glfwSetMouseButtonCallback(window, mouseButtonsCallback);
    // try to enable raw mouse motion, only takes effect when the cursor is disabled
    if (glfwRawMouseMotionSupported()) glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    else fprintf(stderr, "[WARNING] Failed to enable raw mouse motion.\n");

    //initializing GLAD
    if (!gladLoadGLES2Loader((GLADloadproc)glfwGetProcAddress))
    {
        fprintf(stderr, "GLAD failed to initialize!\n");
        glfwTerminate();
        return 3;
    }

    windowResizeCallback(window, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);

    //Inits
    if (!Meshes::initBasicMeshes())
    {
        fprintf(stderr, "Failed to initialize basic meshes!\n");
        glfwTerminate();
        return 4;
    }

    //Camera
    using Camera = Drawing::Camera3D;

    const float mouse_sens = 0.08f;
    float fov = 80.f;
    const glm::vec3 camera_init_pos(0.f, 0.7f, 2.5f);
    //const glm::vec3 camera_init_target = camera_init_pos + glm::vec3(0.f, 0.f, -1.f);
    //Camera camera(fov, (float)window_width / (float)window_height, camera_init_pos, camera_init_target);
    float camera_pitch = 0.f;
    float camera_yaw = -90.f;
    Camera camera(fov, (float)window_width / (float)window_height, camera_init_pos, camera_pitch, camera_yaw);

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

    Meshes::VBO cube_vbo(cube_vertices, cube_vert_count, true, true);
    if (cube_vbo.m_id == Meshes::empty_id)
    {
        fprintf(stderr, "Failed to create cube VBO!\n");
        glfwTerminate();
        return 4;
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
    
    unsigned int line_vbo;
    glGenBuffers(1, &line_vbo);
    if (!line_vbo)
    {
        fprintf(stderr, "Failed to create unit line VBO!\n");
        glfwTerminate();
        return 4;
    }

    glBindBuffer(GL_ARRAY_BUFFER, line_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(line_vertices), line_vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind the buffer afterwards

    //Textures
    using Texture = Textures::Texture2D;

    //TODO those values
    unsigned int fbo3d_init_width = DEFAULT_WINDOW_WIDTH, fbo3d_init_height = DEFAULT_WINDOW_HEIGHT;
    Texture fbo3d_tex(fbo3d_init_width, fbo3d_init_height, GL_RGB);
    if (fbo3d_tex.m_id == Textures::empty_id)
    {
        fprintf(stderr, "Failed to create FrameBuffer color texture for 3D rendering!\n");
        glfwTerminate();
        return 5;
    }

    const char *bricks_path = "assets/bricks2.png";
    const glm::vec2 brick_texture_world_size(0.75f, 0.75f); // aspect ration 1:1

    Texture brick_texture(bricks_path);
    if (brick_texture.m_id == Textures::empty_id)
    {
        fprintf(stderr, "Failed to create brick texture!\n");
        glfwTerminate();
        return 5;
    }

    const char *bricks_alt_path = "assets/bricks1.jpg";
    const glm::vec2 brick_alt_texture_world_size(0.75f, 0.75f); // aspect ration 1:1

    Texture brick_alt_texture(bricks_alt_path);
    if (brick_alt_texture.m_id == Textures::empty_id)
    {
        fprintf(stderr, "Failed to create brick alternative texture!\n");
        glfwTerminate();
        return 5;
    }

    // const char *orb_path = "assets/orb.jpg";
    // const glm::vec2 orb_texture_world_size(1.f, 1.f); // almost 1:1 aspect ratio

    // Texture orb_texture(orb_path);
    // if (orb_texture.m_id == Textures::empty_id)
    // {
    //     fprintf(stderr, "Failed to create orb texture!\n");
    //     glfwTerminate();
    //     return 5;
    // }

    const char *target_path = "assets/target_small.png";
    const glm::vec2 target_texture_world_size(1.f, 1.f); // 1:1 aspect ratio, size itself does not matter really
    const float target_texture_dish_radius = 0.915f / 2.f; // radius of the target dish compared to the size of the full image (1.0x1.0)

    Texture target_texture(target_path);
    if (target_texture.m_id == Textures::empty_id)
    {
        fprintf(stderr, "Failed to create target texture!\n");
        glfwTerminate();
        return 5;
    }

    //RenderBuffers
    //TODO render buffer object abstraction
    GLuint fbo3d_rbo_depth = 0, fbo3d_rbo_stencil = 0;
    {
        GLuint rbos[2];
        glGenRenderbuffers(2, rbos);
        fbo3d_rbo_depth = rbos[0];
        fbo3d_rbo_stencil = rbos[1];
    }

    //depth renderbuffer allocation
    glBindRenderbuffer(GL_RENDERBUFFER, fbo3d_rbo_depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, fbo3d_tex.m_width, fbo3d_tex.m_height);

    //depth renderbuffer allocation
    glBindRenderbuffer(GL_RENDERBUFFER, fbo3d_rbo_stencil);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, fbo3d_tex.m_width, fbo3d_tex.m_height);

    glBindRenderbuffer(GL_RENDERBUFFER, 0); //TODO empty_id

    //Shaders
    using ShaderP = Shaders::Program;

    const char *default_vs_path = "shaders/default.vs",
               *default_fs_path = "shaders/default.fs",
            //    *passthrough_pos_vs_path = "shaders/passthrough-pos.vs",
            //    *passthrough_pos_uv_vs_path = "shaders/passthrough-pos-uv.vs",
               *transform_vs_path = "shaders/transform.vs",
               *static_color_fs_path = "shaders/static-color.fs";

    //line shader
    const char *screen_line_vs_path = "shaders/screen2d-line.vs";

    ShaderP screen_line_shader(screen_line_vs_path, static_color_fs_path);
    if (screen_line_shader.m_id == Shaders::empty_id)
    {
        fprintf(stderr, "Failed to create screen line shader program!\n");
        glfwTerminate();
        return 6;
    }

    //ui shader
    const char *ui_vs_path = "shaders/ui.vs",
               *ui_fs_path = "shaders/ui.fs";
    
    ShaderP ui_shader(ui_vs_path, ui_fs_path);
    if (ui_shader.m_id == Shaders::empty_id)
    {
        fprintf(stderr, "Failed to create UI shader program!\n");
        glfwTerminate();
        return 6;
    }

    //textured rectangle shader
    const char *tex_rect_fs_path = "shaders/tex-rect.fs";

    ShaderP tex_rect_shader(transform_vs_path, tex_rect_fs_path);
    if (tex_rect_shader.m_id == Shaders::empty_id)
    {
        fprintf(stderr, "Failed to create textured rectangle shader program!\n");
        glfwTerminate();
        return 6;
    }

    //Light sources
    // loading special light source shader program (renders light source without light effects etc.)
    const char *light_src_fs_path = "shaders/light_src.fs";
    
    ShaderP light_src_shader(default_vs_path, light_src_fs_path); // using the default vertex shader
    if (light_src_shader.m_id == Shaders::empty_id)
    {
        fprintf(stderr, "Failed to create light source shader program!\n");
        glfwTerminate();
        return 6;
    }

    // loading light shader program
    const char *light_vs_path = "shaders/texture.vs";
    const char *light_fs_path = "shaders/light.fs";

    ShaderP light_shader(light_vs_path, light_fs_path); // using the texture vertex shader (maybe change this?)
    if (light_shader.m_id == Shaders::empty_id)
    {
        fprintf(stderr, "Failed to create shader program for lighting!\n");
        glfwTerminate();
        return 6;
    }

    const float light_src_size = 0.2;
    using LightProps = Lighting::LightProps;
    using DirLight = Lighting::DirLight;
    using PointLight = Lighting::PointLight;
    using SpotLight = Lighting::SpotLight;

    //sun
    const LightProps sun_light_props(Color3F(0.9f, 0.8f, 0.7f), 0.4f);
    DirLight sun(sun_light_props, glm::vec3(1.f, -1.f, -0.5f));

    //flashlight
    const Color3F flashlight_color = Color3F(1.f, 1.f, 1.f);
    const LightProps flashlight_light_props(flashlight_color, 0.f);

    SpotLight flashlight(flashlight_light_props, glm::vec3(0.f), glm::vec3(0.f), //throwaway values for position and direction
                         40.f, 50.f);
    flashlight.setAttenuation(1.f, 0.09f, 0.032f);
    bool show_flashlight = false;

    //Materials
    using MaterialProps = Lighting::MaterialProps;

    MaterialProps default_material(Color3F(1.0f, 1.0f, 1.0f), 32.f);

    //UI
    const char *font_path = "assets/DINEngschrift-Regular.ttf";
    const float font_size = 22;

    //TODO load stuff into textbuffer
    unsigned int textbuffer[UNICODE_TEXTBUFFER_LEN] = {};
    size_t textbuffer_len = 0;

    const UI::Font font(font_path, font_size);
    if (font.getFontPtr() == NULL)
    {
        fprintf(stderr, "Failed to initialize UI font!\n");
        return 7;
    }

    UI::Context ui(ui_shader, font);
    if (!ui.m_ctx_initialized)
    {
        fprintf(stderr, "Failed to initialize UI!\n");
        return 7;
    }

    //Framebuffers
    using FrameBuffer = Drawing::FrameBuffer;
    //TODO
    FrameBuffer fbo3d{}; // need to use curly braces as for function declaration disambiguation
    if (fbo3d.m_id == 0) //TODO empty_id
    {
        fprintf(stderr, "Failed to initialize FrameBuffer for 3D scene!\n");
        return 8;
    }

    fbo3d.attachAll(fbo3d_tex.asFrameBufferAttachment(),
                    FrameBuffer::Attachment{ fbo3d_rbo_depth, FrameBuffer::AttachmentType::render },
                    FrameBuffer::Attachment{ fbo3d_rbo_stencil, FrameBuffer::AttachmentType::render });
    
    if (!fbo3d.isComplete())
    {
        fprintf(stderr, "FrameBuffer for 3D scene is not complete!\n");
        return 8;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0); //TODO empty_id // unbind just in case

    //Wall and it's vbo
    const glm::vec3 wall_size(5.f, 2.5f, 0.2f);
    const glm::vec3 wall_pos(0.f, wall_size.y / 2.f, 0.f);
    
    Meshes::VBO wall_vbo = Meshes::generateCubicVBO(wall_size, brick_alt_texture_world_size,
                                                    Meshes::TexcoordStyle::repeat, true);
    if (wall_vbo.m_id == Meshes::empty_id)
    {
        fprintf(stderr, "Failed to create wall VBO!\n");
        glfwTerminate();
        return 9;
    }

    //Targets
    using Target = Game::Target;

    Meshes::VBO target_vbo = Meshes::generateQuadVBO(glm::vec2(1.f), target_texture_world_size,
                                                     Meshes::TexcoordStyle::stretch, true);
    if (target_vbo.m_id == Meshes::empty_id)
    {
        fprintf(stderr, "Failed to create target's VBO!\n");
        glfwTerminate();
        return 9;
    }

    glm::vec3 target_pos_offset(0.f, wall_size.y / 2.f, wall_size.z / 2.f);
    std::vector<Target> targets;

    //targets rng init
    Utils::RNG target_rng_width(-1000, 1000);
    Utils::RNG target_rng_height(-500, 500);

    //Level variables
    const double level_spawn_rate_init = 0.6f; // target per second
    const double level_spawn_rate_mult = 1.35f;
    const size_t level_amount_init = 8;
    const size_t level_amount_inc = 4;

    double target_last_spawn_time = -1.f; // -1 should get us immediate first spawn, maybe use tick == 0 instead?
    double level_spawn_rate = level_spawn_rate_init;
    size_t level = 1;
    size_t level_targets_hit = 0;

    //Misc.
    Color clear_color_3d(50, 220, 80), clear_color_2d(0, 0, 0);
    unsigned int tick = 0;
    float frame_delta = 0.f;
    double last_frame_time = glfwGetTime();
    double last_mouse_x = 0.f, last_mouse_y = 0.f;
    bool last_left_mbutton = false;
    
    while(!glfwWindowShouldClose(window))
    {
        //calculating correct frame delta time
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

        if (mouse_delta_x != 0.f || mouse_delta_y != 0.f)
        {
            camera_yaw += mouse_sens * mouse_delta_x;
            camera_pitch = std::min(89.f, std::max(-89.f, camera_pitch - mouse_sens * mouse_delta_y)); // set camera_pitch with limits -89°/89°
            camera.setTargetFromPitchYaw(camera_pitch, camera_yaw); //TODO make this better
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
        if (mbutton_left_is_clicked)
        {
            //puts("Fire!");
            for (size_t i = 0; i < targets.size(); ++i)
            {
                size_t idx = targets.size() - 1 - i; // iterating from the back so the closest targets get hit first

                glm::vec3 pos_offset = glm::vec3(0.f, 0.f, FLOAT_TOLERANCE * (i + 1)); // adding offset so we reduce z-fighting
                glm::vec3 pos = targets[idx].m_pos + pos_offset;
                glm::vec2 size = targets[idx].getSize(current_frame_time);
                assert(size.x == size.y);
                float radius = size.x * target_texture_dish_radius;

                Collision::RayCollision rcoll = Collision::rayTarget(mouse_ray, glm::vec3(0.f, 0.f, 1.f), pos, radius);
                if (rcoll.m_hit)
                {
                    targets.erase(targets.begin() + idx);
                    
                    ++level_targets_hit;
                    size_t level_target_amount = level_amount_init + (level - 1) * level_amount_inc;
                    printf("Hit (%d)! Targets hit: %d/%d\n", idx, level_targets_hit, level_target_amount);

                    assert(level_targets_hit <= level_target_amount);
                    if (level_targets_hit >= level_target_amount)
                    {
                        printf("Level %d completed!\n", level);
                        ++level;
                        level_targets_hit = 0;
                        level_spawn_rate *= level_spawn_rate_mult;
                    }

                    break;
                }
            }
        }

        // ---Target spawning---
        {
            double time_to_spawn = 1.f / level_spawn_rate;

            if (current_frame_time - target_last_spawn_time >= time_to_spawn)
            {
                glm::vec2 wall_spawnable_area = glm::vec2(wall_size.x, wall_size.y) - glm::vec2(target_texture_dish_radius * Target::size_max * 2.f); // *2 for both borders
                glm::vec3 pos = target_pos_offset + Game::Target::generateXZPosition(target_rng_width, target_rng_height, wall_spawnable_area);
                targets.emplace_back(target_vbo, target_texture, default_material, pos, current_frame_time);

                //NOTE simple solution, might produce slower spawn rates on inconsistent/low fps
                target_last_spawn_time = current_frame_time;
            }
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

            // lights.emplace_back(&flashlight);
            lights.push_back(flashlight);
        }

        // ---UI---
        //pump the input into UI
        if (!ui.getInput(window, mouse_pos, mbutton_left_is_pressed, textbuffer, textbuffer_len))
        {
            fprintf(stderr, "[WARNING] Failed to update the input for UI!\n");
        }

        //TODO GUI
        /*if (nk_begin(&ui.m_ctx, "Title", nk_rect(100, 50, 420, 520),
            NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_MOVABLE | NK_WINDOW_CLOSABLE))
        {
            nk_layout_row_dynamic(&ui.m_ctx, 50, 1);

            nk_label(&ui.m_ctx, "text", NK_TEXT_CENTERED);
            if (nk_button_label(&ui.m_ctx, "BACK"))
            {
                puts("BACK");
            }
            nk_label(&ui.m_ctx, "text", NK_TEXT_CENTERED);

            if (nk_button_label(&ui.m_ctx, "BACK2"))
            {
                puts("BACK2");
            }
            nk_label(&ui.m_ctx, "textXX", NK_TEXT_CENTERED);

            nk_layout_row_dynamic(&ui.m_ctx, 50, 1);
            nk_label(&ui.m_ctx, "text22", NK_TEXT_CENTERED);

            // fixed widget pixel width
            // nk_layout_row_dynamic(&ui.m_ctx, 50, 3);
            // if (nk_button_label(&ui.m_ctx, "button"))
            // {
            //     puts("button pressed");
            // }
            // if (nk_button_label(&ui.m_ctx, "button2"))
            // {
            //     puts("back to game");
            // }
            // if (nk_button_label(&ui.m_ctx, "button3"))
            // {
            //     puts("back to game");
            // }

            // fixed widget window ratio width
            // nk_layout_row_dynamic(&ui.m_ctx, 30, 2);
            // if (nk_option_label(&ui.m_ctx, "true", true))
            // {
            //     // puts("true switched");
            // }
            // if (nk_option_label(&ui.m_ctx, "false", false))
            // {
            //     puts("false switched");
            // }
        }
        nk_end(&ui.m_ctx);*/

        if (nk_begin_titled(&ui.m_ctx, "Name", "Title", nk_rect(50, 20, 200, 250),
            NK_WINDOW_BORDER | NK_WINDOW_TITLE))
        {
            //TODO
            // nk_layout_row_static(&ui.m_ctx, 30, 80, 2);
            nk_layout_row_dynamic(&ui.m_ctx, 50, 1);
            if (nk_button_label(&ui.m_ctx, "a")) {
                puts("button1 pressed");
            }
            //nk_label(&ui.m_ctx, "text", NK_TEXT_CENTERED);
            // if (nk_button_label(&ui.m_ctx, "button2")) {
            //     puts("button2 pressed");
            // }
        }
        nk_end(&ui.m_ctx);

        // ---Draw begin---
        {
            //3D block
            {
                //set the viewport according to wanted framebuffer
                glViewport(0, 0, fbo3d_tex.m_width, fbo3d_tex.m_height);

                //bind the correct framebuffer
                fbo3d.bind();
                Drawing::clear(clear_color_3d);
                glClear(GL_DEPTH_BUFFER_BIT); //TODO make this nicer - probably move into Drawing

                const glm::mat4& view_mat = camera.getViewMatrix();
                const glm::mat4& proj_mat = camera.getProjectionMatrix();

                glEnable(GL_DEPTH_TEST);

                //targets (no face culling for them)
                for (size_t i = 0; i < targets.size(); ++i)
                {
                    glm::vec3 pos_offset = glm::vec3(0.f, 0.f, FLOAT_TOLERANCE * (i + 1)); // adding offset so we reduce z-fighting
                    targets[i].draw(light_shader, camera, lights, current_frame_time, pos_offset);
                }

                //Enable backface culling
                glCullFace(GL_BACK);
                glEnable(GL_CULL_FACE);

                //cube
                light_shader.use();
                brick_texture.bind();
                {
                    glm::vec3 pos = //wall_rcoll.m_hit ? wall_rcoll.m_point :
                                    glm::vec3(-4.f, 0.35f, -0.5f); //DEBUG

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
                    light_shader.setMaterialProps(default_material);
                    light_shader.setLights(UNIFORM_LIGHT_NAME, UNIFORM_LIGHT_COUNT_NAME, lights); // return value ignored here
                }

                cube_vbo.bind();
                    glDrawArrays(GL_TRIANGLES, 0, cube_vbo.m_vert_count);
                cube_vbo.unbind();

                //wall
                light_shader.use();
                // brick_texture.bind();
                brick_alt_texture.bind();
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
                    light_shader.setMaterialProps(default_material);
                    light_shader.setLights(UNIFORM_LIGHT_NAME, UNIFORM_LIGHT_COUNT_NAME, lights); // return value ignored here
                }

                wall_vbo.bind();
                    glDrawArrays(GL_TRIANGLES, 0, wall_vbo.m_vert_count);
                wall_vbo.unbind();
                

                glDisable(GL_CULL_FACE);
                glDisable(GL_DEPTH_TEST);
            }

            //2D block
            {
                glm::vec2 window_res((float)window_width, (float)window_height);
                glm::vec2 window_middle = window_res / 2.f;

                //TODO this might be wrong on some displays?
                //set the viewport according to window size
                glViewport(0, 0, window_res.x, window_res.y);

                //bind the default framebuffer
                glBindFramebuffer(GL_FRAMEBUFFER, 0); //TODO empty_id
                Drawing::clear(clear_color_2d);
                //TODO maybe useless in 2D block?
                glClear(GL_DEPTH_BUFFER_BIT); //TODO make this nicer - probably move into Drawing

                glDisable(GL_CULL_FACE);
                glEnable(GL_BLEND); //TODO check this
                glBlendEquation(GL_FUNC_ADD); //TODO check this
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //TODO check this

                //render the 3D scene as a background from it's framebuffer
                Drawing::texturedRectangle(tex_rect_shader, fbo3d_tex, glm::vec2(0.f), window_res);

                //line test
                // Drawing::screenLine(screen_line_shader, line_vbo, window_res,
                //                     screen_middle, glm::vec2(50.f),
                //                     50.f, ColorF(1.0f, 0.0f, 0.0f));

                //crosshair
                const ColorF crosshair_color = ColorF(1.f, 1.f, mbutton_left_is_pressed ? 1.f : 0.f);
                Drawing::crosshair(screen_line_shader, line_vbo, window_res,
                                   glm::vec2(50.f, 30.f), window_middle, 1.f, crosshair_color);

                //UI drawing
                glEnable(GL_SCISSOR_TEST); // enable scissor for UI drawing only
                {
                    if (!ui.draw(window_res))
                    {
                        fprintf(stderr, "[WARNING] Failed to draw the UI!\n");
                    }
                    ui.clear();
                }
                glDisable(GL_SCISSOR_TEST);

                glDisable(GL_BLEND);
            }
        }

        // ---Draw end---
        //glfwPollEvents();
        glfwSwapBuffers(window);

        last_mouse_x = mouse_x;
        last_mouse_y = mouse_y;
        last_left_mbutton = left_mbutton_state;
        ++tick;
    }

    glDeleteBuffers(1, &line_vbo);
    glDeleteRenderbuffers(1, &fbo3d_rbo_depth);
    glDeleteRenderbuffers(1, &fbo3d_rbo_stencil);

    glfwTerminate();
    puts("Game end.");

    return 0;
}

int main(void)
{
    return game_main();
    // return test_main();
}
