#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "game.hpp"

#include "glm/ext/matrix_transform.hpp" //glm::lookAt
#include "glm/gtc/matrix_transform.hpp" //glm::perspective

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

//USELESS
//helper function for binding lights to shaders
/*void bufferAttributeWrite(char *buffer, size_t buffer_len, const char *attr, size_t attr_len)
{
    //buffer is expected to be buffer_len + 1 long! (because of term. char)
    assert(1 + attr_len <= buffer_len);     // +1 is for dot 
    
    buffer[0] = '.'; // (here is the dot)
    strncpy(buffer + 1, attr, attr_len);    // +1 is for dot
    buffer[1 + attr_len] = '\0';            // +1 is for dot
}*/

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

void Drawing::crosshair(glm::vec2 size, glm::vec2 screen_pos, float thickness, Color3F color)
{
    //TODO
    
}
