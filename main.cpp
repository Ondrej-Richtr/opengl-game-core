#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "game.hpp"
#include "stb_image.h"

#include "glm/gtc/matrix_transform.hpp" //DEBUG

#include <algorithm>

//TODO move this elsewhere - window manager?
void windowResizeCallback(GLFWwindow* window, int width, int height)
{
    //TODO check for resizes to 0x0?
    printf("Window resized to: %dx%d\n", width, height);
    glViewport(0, 0, width, height);
}

// double last_xpos = 0.f, last_ypos = 0.f;
// void windowMouseMoveCallback(GLFWwindow* window, double xpos, double ypos)
// {
//     //TODO
//     const float sensitivity = 0.1f;
//     printf("xpos: %f, ypos: %f, last_xpos: %f, last_ypos: %f\n", xpos, ypos, last_xpos, last_ypos);
//     last_xpos = xpos;
//     last_ypos = ypos;
//     float dif_x = (float)(xpos - last_xpos), dif_y = (float)(ypos - last_ypos);
//     //printf("xpos: %f, ypos: %f, last_xpos: %f, last_ypos: %f\n", xpos, ypos, last_xpos, last_ypos);
// }

int main()
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
    int window_width = DEFAULT_WINDOW_WIDTH, window_height = DEFAULT_WINDOW_HEIGHT;
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

    glViewport(0, 0, window_width, window_height);

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
    Color clear_color(50, 220, 80);
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
        //positions             //texcoords
        0.2f,  0.2f,   0.0f,     1.0f, 1.0f, // top right
        0.2f,  -0.2f,  0.0f,     1.0f, 0.0f, // bottom right
        -0.2f,  -0.2f, 0.0f,     0.0f, 0.0f, // bottom left
        -0.2f,  0.2f,  0.0f,     0.0f, 1.0f  // top left 
    };
    GLuint square_indices[] = {
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };
    size_t square_verts_pos_offset = 0;
    size_t square_verts_texcoord_offset = 3;
    size_t square_vert_attrib = 5; //amount of square attributes - 3x pos + 2x texcoords

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
        //pos                //texcoords
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };
    size_t cube_verts_pos_offset = 0;
    size_t cube_verts_texcoord_offset = 3;
    size_t cube_vert_attrib = 5; //amount of cube attributes - 3x pos + 2x texcoords

    unsigned int cube_vbo; //TODO make this better
    glGenBuffers(1, &cube_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //Default shader program
    using ShaderP = Shaders::Program;

    const char *default_vs_src_path = "shaders/default.vs",
               *default_fs_src_path = "shaders/default.fs";
    
    ShaderP default_shader(default_vs_src_path, default_fs_src_path);
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

    // MainLoopData loopData = {}; //TODO use this
    // loopData.test = 15;
    // loopData.window = window;

    //Misc.
    unsigned int tick = 0;
    float frame_delta = 0.f;
    double last_frame_time = glfwGetTime();
    double last_mouse_x = 0.f, last_mouse_y = 0.f;
    
    while(!glfwWindowShouldClose(window))
    {
        //calculating correct frame delta time
        //TODO
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

        // ---Draw begin---
        {
            Drawing::clear(window, clear_color);
            glClear(GL_DEPTH_BUFFER_BIT); //TODO make this nicer - probably move into Drawing

            const glm::mat4& view_mat = camera.getViewMatrix();
            const glm::mat4& proj_mat = camera.getProjectionMatrix();

            //triangle
            default_shader.use();
            {
                float time = glfwGetTime();
                float blue_value = sin(time) / 2.0f + 0.5f;
                default_shader.set("inputColor", { 0.0f, 0.0f, blue_value, 1.0f });

                glm::mat4 model_mat = glm::mat4(1.0f);
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

            //square
            texture_shader.use();
            brick_texture.bind(0);
            orb_texture.bind(1);
            {
                glm::mat4 model_mat = glm::mat4(1.f);
                model_mat = glm::translate(model_mat, glm::vec3(0.7f, 0.7f, 0.f));

                texture_shader.set("model", model_mat);
                texture_shader.set("view", view_mat);
                texture_shader.set("projection", proj_mat);
            }

            glBindBuffer(GL_ARRAY_BUFFER, square_vbo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, square_ebo);
                Shaders::setupVertexAttribute_float(0, 3, square_verts_pos_offset, square_vert_attrib * sizeof(GLfloat));
                Shaders::setupVertexAttribute_float(1, 2, square_verts_texcoord_offset, square_vert_attrib * sizeof(GLfloat));
                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                Shaders::disableVertexAttribute(0);
                Shaders::disableVertexAttribute(1);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

            //3D block
            {
                glEnable(GL_DEPTH_TEST);

                //cube
                texture_shader.use();
                brick_texture.bind(0);
                orb_texture.bind(1);
                {
                    float time = glfwGetTime();
                    glm::mat4 model_mat = glm::mat4(1.0f);
                    model_mat = glm::scale(model_mat, glm::vec3(0.5f, 0.5f, 0.5f));
                    model_mat = glm::rotate(model_mat, time, glm::vec3(0.f, 1.f, 0.f));
                    model_mat = glm::rotate(model_mat, time, glm::vec3(0.f, 0.f, 1.f));

                    texture_shader.set("model", model_mat);
                    texture_shader.set("view", view_mat);
                    texture_shader.set("projection", proj_mat);
                }

                glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
                    Shaders::setupVertexAttribute_float(0, 3, cube_verts_pos_offset, cube_vert_attrib * sizeof(GLfloat));
                    Shaders::setupVertexAttribute_float(1, 2, cube_verts_texcoord_offset, cube_vert_attrib * sizeof(GLfloat));
                        glDrawArrays(GL_TRIANGLES, 0, 36);
                    Shaders::disableVertexAttribute(0);
                    Shaders::disableVertexAttribute(1);
                glBindBuffer(GL_ARRAY_BUFFER, 0);

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