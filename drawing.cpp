#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "game.hpp"

#include "glm/ext/matrix_transform.hpp" //glm::lookAt
#include "glm/gtc/matrix_transform.hpp" //glm::perspective
#include "glm/gtx/matrix_transform_2d.hpp" //glm::translate and glm::scale

#include <cstring>


Drawing::Camera3D::Camera3D(float fov, float aspect_ration, glm::vec3 pos, glm::vec3 target,
                            float near_plane, float far_plane)
                    : m_pos(pos), m_target(target), m_proj_mat(), m_view_mat()
{
    updateViewMatrix(); // properly sets m_view_mat
    m_proj_mat = glm::perspective(glm::radians(fov), aspect_ration, near_plane, far_plane);
}

Drawing::Camera3D::Camera3D(float fov, float aspect_ration, glm::vec3 pos, float pitch, float yaw,
                            float near_plane, float far_plane)
                    : m_pos(pos), m_target(), m_proj_mat(), m_view_mat()
{
    setTargetFromPitchYaw(pitch, yaw);  // properly sets m_target and m_view_mat
    //updateViewMatrix();               // properly sets m_view_mat
    m_proj_mat = glm::perspective(glm::radians(fov), aspect_ration, near_plane, far_plane);
}

void Drawing::Camera3D::setPosition(glm::vec3 pos)
{
    if (pos != m_pos)
    {
        m_pos = pos;
        updateViewMatrix();
    }
}

void Drawing::Camera3D::movePosition(glm::vec3 move_vec)
{
    setPosition(m_pos + move_vec);
}

void Drawing::Camera3D::setTarget(glm::vec3 target)
{
    if (target != m_target)
    {
        m_target = target;
        updateViewMatrix();
    }
}

void Drawing::Camera3D::moveTarget(glm::vec3 move_vec)
{
    setTarget(m_target + move_vec);
}

void Drawing::Camera3D::setTargetFromPitchYaw(float pitch, float yaw)
{
    //TODO
    const float sin_pitch = sin(glm::radians(pitch));
    const float cos_pitch = cos(glm::radians(pitch));
    const float sin_yaw = sin(glm::radians(yaw));
    const float cos_yaw = cos(glm::radians(yaw));
    glm::vec3 dir = glm::normalize(glm::vec3(cos_yaw * cos_pitch, sin_pitch, sin_yaw * cos_pitch)); //TODO up_vec

    setTarget(m_pos + dir);
}

void Drawing::Camera3D::move(glm::vec3 move_vec)
{
    movePosition(move_vec);
    moveTarget(move_vec);
}

// glm::vec3 Drawing::Camera3D::coordsWorldToView(glm::vec3 vec) const
// {
//     return getViewMatrix() * vec;
// }

void Drawing::Camera3D::updateViewMatrix()
{
    m_view_mat = glm::lookAt(m_pos, m_target, Drawing::up_dir);
}

const glm::mat4& Drawing::Camera3D::getViewMatrix() const
{
    return m_view_mat;
}

const glm::mat4& Drawing::Camera3D::getProjectionMatrix() const
{
    return m_proj_mat;
}

glm::vec3 Drawing::Camera3D::dirCoordsViewToWorld(glm::vec3 dir) const
{
    glm::mat3 m(getViewMatrix()); //IDEA cache this too
    glm::vec3 dir_transformed = glm::inverse(m) * dir;

    return NORMALIZE_OR_0(dir_transformed);
}

glm::vec3 Drawing::Camera3D::getDirection() const
{
    return NORMALIZE_OR_0(m_target - m_pos);
}

Collision::Ray Drawing::Camera3D::getRay() const
{
    return Collision::Ray(m_pos, getDirection());
}

Drawing::Light::Light(const LightProps& props)
                        : m_props(props) {}

bool Drawing::Light::bindPropsToShader(const char *uniform_name, const Shaders::Program& shader, int idx) const
{
    // bind the light props (ambient, diffuse, specular) to given shader under given uniform_name,
    // if idx is non-negative then it appends array access after the uniform name (as "[idx]"),
    // returns whether binding was successful
    char str_buffer[UNIFORM_NAME_BUFFER_LEN + 1];
    
    //ambient
    size_t len = snprintf((char*)str_buffer, UNIFORM_NAME_BUFFER_LEN + 1,
                           idx >= 0 ? "%s[%d]." UNIFORM_LIGHTPROPS_ATTRNAME "." UNIFORM_LIGHTPROPS_AMBIENT
                                    : "%s."     UNIFORM_LIGHTPROPS_ATTRNAME "." UNIFORM_LIGHTPROPS_AMBIENT,
                           uniform_name, idx);
    // negative len indicates that there was an error, len > buffer len indicates that buffer was not big enough
    if (len < 0 || len > UNIFORM_NAME_BUFFER_LEN) return false; 
    
    shader.set(str_buffer, m_props.m_ambient);

    //diffuse
    len = snprintf((char*)str_buffer, UNIFORM_NAME_BUFFER_LEN + 1,
                   idx >= 0 ? "%s[%d]." UNIFORM_LIGHTPROPS_ATTRNAME "." UNIFORM_LIGHTPROPS_DIFFUSE
                            : "%s."     UNIFORM_LIGHTPROPS_ATTRNAME "." UNIFORM_LIGHTPROPS_DIFFUSE,
                   uniform_name, idx);
    // negative len indicates that there was an error, len > buffer len indicates that buffer was not big enough
    if (len < 0 || len > UNIFORM_NAME_BUFFER_LEN) return false; 
    
    shader.set(str_buffer, m_props.m_diffuse);

    //specular
    len = snprintf((char*)str_buffer, UNIFORM_NAME_BUFFER_LEN + 1,
                   idx >= 0 ? "%s[%d]." UNIFORM_LIGHTPROPS_ATTRNAME "." UNIFORM_LIGHTPROPS_SPECULAR
                            : "%s."     UNIFORM_LIGHTPROPS_ATTRNAME "." UNIFORM_LIGHTPROPS_SPECULAR,
                   uniform_name, idx);
    // negative len indicates that there was an error, len > buffer len indicates that buffer was not big enough
    if (len < 0 || len > UNIFORM_NAME_BUFFER_LEN) return false; 
    
    shader.set(str_buffer, m_props.m_specular);

    return true;
}

Drawing::DirLight::DirLight(const LightProps& props, glm::vec3 dir)
                            : Light(props), m_dir(glm::normalize(dir)) {}

bool Drawing::DirLight::bindToShader(const char *uniform_name, const Shaders::Program& shader, int idx) const
{
    // binds the props, type and direction to given shader under given uniform_name, other attributes are left unchanged/unbinded,
    // if idx is non-negative then it appends array access after the uniform name (as "[idx]"),
    // returns whether binding was successful
    bool props_bind = bindPropsToShader(uniform_name, shader, idx);
    if (!props_bind) return false;

    char str_buffer[UNIFORM_NAME_BUFFER_LEN + 1];
    
    //type
    size_t len = snprintf((char*)str_buffer, UNIFORM_NAME_BUFFER_LEN + 1,
                           idx >= 0 ? "%s[%d]." UNIFORM_LIGHT_TYPE
                                    : "%s."     UNIFORM_LIGHT_TYPE,
                           uniform_name, idx);
    // negative len indicates that there was an error, len > buffer len indicates that buffer was not big enough
    if (len < 0 || len > UNIFORM_NAME_BUFFER_LEN) return false; 
    
    shader.set(str_buffer, static_cast<GLint>(Drawing::Light::Type::directional));

    //dir
    len = snprintf((char*)str_buffer, UNIFORM_NAME_BUFFER_LEN + 1,
                   idx >= 0 ? "%s[%d]." UNIFORM_LIGHT_DIRECTION
                            : "%s."     UNIFORM_LIGHT_DIRECTION,
                   uniform_name, idx);
    // negative len indicates that there was an error, len > buffer len indicates that buffer was not big enough
    if (len < 0 || len > UNIFORM_NAME_BUFFER_LEN) return false; 
    
    shader.set(str_buffer, m_dir);

    return true;
}

Drawing::PointLight::PointLight(const LightProps& props, glm::vec3 pos)
                                : Light(props), m_pos(pos) {}

bool Drawing::PointLight::bindToShader(const char *uniform_name, const Shaders::Program& shader, int idx) const
{
    // binds the props, type and position to given shader under given uniform_name, other attributes are left unchanged/unbinded,
    // if idx is non-negative then it appends array access after the uniform name (as "[idx]"),
    // returns whether binding was successful
    bool props_bind = bindPropsToShader(uniform_name, shader, idx);
    if (!props_bind) return false;

    char str_buffer[UNIFORM_NAME_BUFFER_LEN + 1];
    
    //type
    size_t len = snprintf((char*)str_buffer, UNIFORM_NAME_BUFFER_LEN + 1,
                           idx >= 0 ? "%s[%d]." UNIFORM_LIGHT_TYPE
                                    : "%s."     UNIFORM_LIGHT_TYPE,
                           uniform_name, idx);
    // negative len indicates that there was an error, len > buffer len indicates that buffer was not big enough
    if (len < 0 || len > UNIFORM_NAME_BUFFER_LEN) return false; 
    
    shader.set(str_buffer, static_cast<GLint>(Drawing::Light::Type::point));

    //pos
    len = snprintf((char*)str_buffer, UNIFORM_NAME_BUFFER_LEN + 1,
                   idx >= 0 ? "%s[%d]." UNIFORM_LIGHT_POSITION
                            : "%s."     UNIFORM_LIGHT_POSITION,
                   uniform_name, idx);
    // negative len indicates that there was an error, len > buffer len indicates that buffer was not big enough
    if (len < 0 || len > UNIFORM_NAME_BUFFER_LEN) return false; 
    
    shader.set(str_buffer, m_pos);

    //attenuation coefs
    len = snprintf((char*)str_buffer, UNIFORM_NAME_BUFFER_LEN + 1,
                   idx >= 0 ? "%s[%d]." UNIFORM_LIGHT_ATTENUATION
                            : "%s."     UNIFORM_LIGHT_ATTENUATION,
                   uniform_name, idx);
    // negative len indicates that there was an error, len > buffer len indicates that buffer was not big enough
    if (len < 0 || len > UNIFORM_NAME_BUFFER_LEN) return false; 
    
    shader.set(str_buffer, glm::vec3(m_attenuation_coefs_const, m_attenuation_coefs_lin, m_attenuation_coefs_quad));

    return true;
}

void Drawing::PointLight::setAttenuation(GLfloat constant, GLfloat linear, GLfloat quadratic)
{
    m_attenuation_coefs_const = constant;
    m_attenuation_coefs_lin = linear;
    m_attenuation_coefs_quad = quadratic;
}

Drawing::SpotLight::SpotLight(const LightProps& props, glm::vec3 dir, glm::vec3 pos,
                              float inner_cutoff_angle, float outer_cutoff_angle) // cutoff angles are expected in degrees
                                : Light(props), m_dir(glm::normalize(dir)), m_pos(pos),
                                  m_cos_in_cutoff(cos(glm::radians(inner_cutoff_angle))),
                                  m_cos_out_cutoff(cos(glm::radians(outer_cutoff_angle)))
{
    // cutoff angles must make sense - inner cant be larger + they must define valid cone
    assert(inner_cutoff_angle <= outer_cutoff_angle);
    assert(inner_cutoff_angle >= 0.f);
    assert(outer_cutoff_angle <= 180.f);
}

bool Drawing::SpotLight::bindToShader(const char *uniform_name, const Shaders::Program& shader, int idx) const
{
    // binds the props, type and position to given shader under given uniform_name, other attributes are left unchanged/unbinded,
    // if idx is non-negative then it appends array access after the uniform name (as "[idx]"),
    // returns whether binding was successful
    bool props_bind = bindPropsToShader(uniform_name, shader, idx);
    if (!props_bind) return false;

    char str_buffer[UNIFORM_NAME_BUFFER_LEN + 1];
    
    //type
    size_t len = snprintf((char*)str_buffer, UNIFORM_NAME_BUFFER_LEN + 1,
                           idx >= 0 ? "%s[%d]." UNIFORM_LIGHT_TYPE
                                    : "%s."     UNIFORM_LIGHT_TYPE,
                           uniform_name, idx);
    // negative len indicates that there was an error, len > buffer len indicates that buffer was not big enough
    if (len < 0 || len > UNIFORM_NAME_BUFFER_LEN) return false; 
    
    shader.set(str_buffer, static_cast<GLint>(Drawing::Light::Type::spot));

    //dir
    len = snprintf((char*)str_buffer, UNIFORM_NAME_BUFFER_LEN + 1,
                   idx >= 0 ? "%s[%d]." UNIFORM_LIGHT_DIRECTION
                            : "%s."     UNIFORM_LIGHT_DIRECTION,
                   uniform_name, idx);
    // negative len indicates that there was an error, len > buffer len indicates that buffer was not big enough
    if (len < 0 || len > UNIFORM_NAME_BUFFER_LEN) return false; 
    
    shader.set(str_buffer, m_dir);

    //pos
    len = snprintf((char*)str_buffer, UNIFORM_NAME_BUFFER_LEN + 1,
                   idx >= 0 ? "%s[%d]." UNIFORM_LIGHT_POSITION
                            : "%s."     UNIFORM_LIGHT_POSITION,
                   uniform_name, idx);
    // negative len indicates that there was an error, len > buffer len indicates that buffer was not big enough
    if (len < 0 || len > UNIFORM_NAME_BUFFER_LEN) return false; 
    
    shader.set(str_buffer, m_pos);

    //cos inner cutoff angle
    len = snprintf((char*)str_buffer, UNIFORM_NAME_BUFFER_LEN + 1,
                   idx >= 0 ? "%s[%d]." UNIFORM_LIGHT_COSINNERCUTOFF
                            : "%s."     UNIFORM_LIGHT_COSINNERCUTOFF,
                   uniform_name, idx);
    // negative len indicates that there was an error, len > buffer len indicates that buffer was not big enough
    if (len < 0 || len > UNIFORM_NAME_BUFFER_LEN) return false; 
    
    shader.set(str_buffer, m_cos_in_cutoff);

    //cos outer cutoff angle
    len = snprintf((char*)str_buffer, UNIFORM_NAME_BUFFER_LEN + 1,
                   idx >= 0 ? "%s[%d]." UNIFORM_LIGHT_COSOUTERCUTOFF
                            : "%s."     UNIFORM_LIGHT_COSOUTERCUTOFF,
                   uniform_name, idx);
    // negative len indicates that there was an error, len > buffer len indicates that buffer was not big enough
    if (len < 0 || len > UNIFORM_NAME_BUFFER_LEN) return false; 
    
    shader.set(str_buffer, m_cos_out_cutoff);

    //attenuation coefs
    len = snprintf((char*)str_buffer, UNIFORM_NAME_BUFFER_LEN + 1,
                   idx >= 0 ? "%s[%d]." UNIFORM_LIGHT_ATTENUATION
                            : "%s."     UNIFORM_LIGHT_ATTENUATION,
                   uniform_name, idx);
    // negative len indicates that there was an error, len > buffer len indicates that buffer was not big enough
    if (len < 0 || len > UNIFORM_NAME_BUFFER_LEN) return false; 
    
    shader.set(str_buffer, glm::vec3(m_attenuation_coefs_const, m_attenuation_coefs_lin, m_attenuation_coefs_quad));

    return true;
}

void Drawing::SpotLight::setAttenuation(GLfloat constant, GLfloat linear, GLfloat quadratic)
{
    m_attenuation_coefs_const = constant;
    m_attenuation_coefs_lin = linear;
    m_attenuation_coefs_quad = quadratic;
}

void Drawing::clear(GLFWwindow* window, Color color)
{
    const ColorF colorf = color.toFloat();

    glClearColor(colorf.r, colorf.g, colorf.b, colorf.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Drawing::screenLine(const Shaders::Program& line_shader, unsigned int line_vbo, glm::vec2 screen_res,
                         glm::vec2 v1, glm::vec2 v2, float thickness, ColorF color)
{
    assert(line_vbo != 0);
    assert(thickness >= 1.f);

    // v * scale + translation = u
    // -> translation = v1
    // -> scale = v2 - translation = v2 - v1
    const glm::vec2 t = v1;
    const glm::vec2 s = v2 - v1;

    glm::mat3 transform(1.f);
    transform = glm::translate(transform, t);
    transform = glm::scale(transform, s);

    glLineWidth(thickness);

    line_shader.use();
    {
        //vs
        line_shader.set("transform", transform);
        line_shader.set("screenRes", screen_res);

        //fs
        line_shader.set("color", color);
    }

    glBindBuffer(GL_ARRAY_BUFFER, line_vbo);
        Shaders::setupVertexAttribute_float(Shaders::attribute_position_verts, 2, 0, 2 * sizeof(GLfloat));
            glDrawArrays(GL_LINES, 0, 2);
        Shaders::disableVertexAttribute(Shaders::attribute_position_verts);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Drawing::crosshair(const Shaders::Program& line_shader, unsigned int line_vbo, glm::vec2 screen_res,
                        glm::vec2 size, glm::vec2 screen_pos, float thickness, ColorF color)
{
    // horizontal (x) line
    glm::vec2 v1_x(screen_pos.x, screen_pos.y - (size.y / 2.f));
    glm::vec2 v2_x(screen_pos.x, screen_pos.y + (size.y / 2.f));
    Drawing::screenLine(line_shader, line_vbo, screen_res, v1_x, v2_x, thickness, color);

    // vertical (y) line
    glm::vec2 v1_y(screen_pos.x - (size.x / 2.f), screen_pos.y);
    glm::vec2 v2_y(screen_pos.x + (size.x / 2.f), screen_pos.y);
    Drawing::screenLine(line_shader, line_vbo, screen_res, v1_y, v2_y, thickness, color);
}

void Drawing::target(const Shaders::Program& shader, const Drawing::Camera3D& camera,
                     const std::vector<std::reference_wrapper<const Drawing::Light>>& lights, const Game::Target& target,
                     double current_frame_time, glm::vec3 pos_offset)
{
    glm::vec2 target_size = target.getSize(current_frame_time);

    shader.use();
    target.m_texture.bind();
    {
        //vs
        glm::mat4 model_mat(1.f);
        model_mat = glm::translate(model_mat, target.m_pos + pos_offset);
        // assumess that target's vbo is a quad facing in z axis
        model_mat = glm::scale(model_mat, glm::vec3(target_size.x, target_size.y, 1.f));

        glm::mat3 normal_mat = Utils::modelMatrixToNormalMatrix(model_mat);

        shader.set("model", model_mat);
        shader.set("normalMat", normal_mat);
        shader.set("view", camera.getViewMatrix());
        shader.set("projection", camera.getProjectionMatrix());

        //fs
        shader.set("cameraPos", camera.m_pos);
        shader.setMaterialProps(target.m_material);
        
        int lights_set = shader.setLights(UNIFORM_LIGHT_NAME, UNIFORM_LIGHT_COUNT_NAME, lights);
        assert(lights_set >= 0);
        assert((size_t)lights_set <= lights.size());
        if ((size_t)lights_set < lights.size())
        {
            fprintf(stderr, "[WARNING] Not all lights were attached to the shader! Wanted amount: %d, set amount: %d\n.", lights.size(), lights_set);
        }
    }

    target.m_vbo.bind();
        glDrawArrays(GL_TRIANGLES, 0, target.m_vbo.m_vert_count);
    target.m_vbo.unbind();
}
