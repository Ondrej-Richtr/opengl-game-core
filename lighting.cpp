#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "game.hpp"

#include "glm/trigonometric.hpp" //glm::radians


Lighting::Light::Light::Light(const LightProps& props)
                        : m_props(props) {}

bool Lighting::Light::bindPropsToShader(const char *uniform_name, const Shaders::Program& shader, int idx) const
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

Lighting::DirLight::DirLight(const LightProps& props, glm::vec3 dir)
                            : Light(props), m_dir(glm::normalize(dir)) {}

bool Lighting::DirLight::bindToShader(const char *uniform_name, const Shaders::Program& shader, int idx) const
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
    
    shader.set(str_buffer, static_cast<GLint>(Lighting::Light::Type::directional));

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

Lighting::PointLight::PointLight(const LightProps& props, glm::vec3 pos)
                                : Light(props), m_pos(pos) {}

bool Lighting::PointLight::bindToShader(const char *uniform_name, const Shaders::Program& shader, int idx) const
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
    
    shader.set(str_buffer, static_cast<GLint>(Lighting::Light::Type::point));

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

void Lighting::PointLight::setAttenuation(GLfloat constant, GLfloat linear, GLfloat quadratic)
{
    m_attenuation_coefs_const = constant;
    m_attenuation_coefs_lin = linear;
    m_attenuation_coefs_quad = quadratic;
}

Lighting::SpotLight::SpotLight(const LightProps& props, glm::vec3 dir, glm::vec3 pos,
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

bool Lighting::SpotLight::bindToShader(const char *uniform_name, const Shaders::Program& shader, int idx) const
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
    
    shader.set(str_buffer, static_cast<GLint>(Lighting::Light::Type::spot));

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

void Lighting::SpotLight::setAttenuation(GLfloat constant, GLfloat linear, GLfloat quadratic)
{
    m_attenuation_coefs_const = constant;
    m_attenuation_coefs_lin = linear;
    m_attenuation_coefs_quad = quadratic;
}
