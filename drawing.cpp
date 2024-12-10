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

void Drawing::Camera3D::updateViewMatrix() //TODO apply this
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

//helper function for binding lights to shaders
void bufferAttributeWrite(char *buffer, size_t buffer_len, const char *attr, size_t attr_len)
{
    //buffer is expected to be buffer_len + 1 long! (because of term. char)
    assert(1 + attr_len <= buffer_len);     // +1 is for dot 
    
    buffer[0] = '.'; // (here is the dot)
    strncpy(buffer + 1, attr, attr_len);    // +1 is for dot
    buffer[1 + attr_len] = '\0';            // +1 is for dot
}

Drawing::Light::Light(const LightProps& props)
                        : m_props(props) {}

void Drawing::Light::bindPropsToShader(const char *props_uniform_name, size_t props_uniform_name_len,
                                       const Shaders::Program& shader) const
{
    //TODO rework this to constexpr
    //(see https://stackoverflow.com/questions/38955940/how-to-concatenate-static-strings-at-compile-time)
    assert(strlen(props_uniform_name) == props_uniform_name_len);

    char str_buffer[UNIFORM_NAME_BUFFER_LEN + 1];
    assert(props_uniform_name_len <= UNIFORM_NAME_BUFFER_LEN);
    strncpy(str_buffer, props_uniform_name, props_uniform_name_len);
    char *buffer_ptr = (char*)str_buffer + props_uniform_name_len;
    size_t buffer_len = UNIFORM_NAME_BUFFER_LEN - props_uniform_name_len;

    //ambient
    size_t ambient_len = STR_LEN(UNIFORM_LIGHTPROPS_AMBIENT);
    bufferAttributeWrite(buffer_ptr, buffer_len, UNIFORM_LIGHTPROPS_AMBIENT, ambient_len);
    shader.set(str_buffer, m_props.m_ambient);

    //diffuse
    size_t diffuse_len = STR_LEN(UNIFORM_LIGHTPROPS_DIFFUSE);
    bufferAttributeWrite(buffer_ptr, buffer_len, UNIFORM_LIGHTPROPS_DIFFUSE, diffuse_len);
    shader.set(str_buffer, m_props.m_diffuse);

    //specular
    size_t specular_len = STR_LEN(UNIFORM_LIGHTPROPS_SPECULAR);
    bufferAttributeWrite(buffer_ptr, buffer_len, UNIFORM_LIGHTPROPS_SPECULAR, specular_len);
    shader.set(str_buffer, m_props.m_specular);
}

Drawing::DirLight::DirLight(const LightProps& props, glm::vec3 dir)
                            : Light(props), m_dir(glm::normalize(dir)) {}

bool Drawing::DirLight::bindToShader(const char *light_uniform_name, size_t light_uniform_name_len,
                                     const char *props_uniform_name, size_t props_uniform_name_len,
                                     const Shaders::Program& shader) const
{
    // binds the props, type and direction to given shader, other attributes are left unchanged/unbinded
    //TODO sprintf (snprintf?)
    bindPropsToShader(props_uniform_name, props_uniform_name_len, shader);

    char str_buffer[UNIFORM_NAME_BUFFER_LEN + 1];
    assert(light_uniform_name_len <= UNIFORM_NAME_BUFFER_LEN);
    strncpy(str_buffer, light_uniform_name, light_uniform_name_len);
    char *buffer_ptr = (char*)str_buffer + light_uniform_name_len;
    size_t buffer_len = UNIFORM_NAME_BUFFER_LEN - light_uniform_name_len;

    //type
    {
        size_t type_len = STR_LEN(UNIFORM_LIGHT_TYPE);
        bufferAttributeWrite(buffer_ptr, buffer_len, UNIFORM_LIGHT_TYPE, type_len);
        shader.set(str_buffer, static_cast<GLint>(Drawing::Light::Type::directional));
    }

    //dir
    {
        size_t dir_len = STR_LEN(UNIFORM_LIGHT_DIRECTION);
        bufferAttributeWrite(buffer_ptr, buffer_len, UNIFORM_LIGHT_DIRECTION, dir_len);
        shader.set(str_buffer, m_dir);
    }

    return true; //TODO
}

Drawing::PointLight::PointLight(const LightProps& props, glm::vec3 pos)
                                : Light(props), m_pos(pos) {}

bool Drawing::PointLight::bindToShader(const char *light_uniform_name, size_t light_uniform_name_len,
                                       const char *props_uniform_name, size_t props_uniform_name_len,
                                       const Shaders::Program& shader) const
{
    // binds the props, type and position to given shader, other attributes are left unchanged/unbinded
    //TODO sprintf (snprintf?)
    bindPropsToShader(props_uniform_name, props_uniform_name_len, shader);

    char str_buffer[UNIFORM_NAME_BUFFER_LEN + 1];
    assert(light_uniform_name_len <= UNIFORM_NAME_BUFFER_LEN);
    strncpy(str_buffer, light_uniform_name, light_uniform_name_len);
    char *buffer_ptr = (char*)str_buffer + light_uniform_name_len;
    size_t buffer_len = UNIFORM_NAME_BUFFER_LEN - light_uniform_name_len;

    //type
    {
        size_t type_len = STR_LEN(UNIFORM_LIGHT_TYPE);
        bufferAttributeWrite(buffer_ptr, buffer_len, UNIFORM_LIGHT_TYPE, type_len);
        shader.set(str_buffer, static_cast<GLint>(Drawing::Light::Type::point));
    }

    //pos
    {
        size_t pos_len = STR_LEN(UNIFORM_LIGHT_POSITION);
        bufferAttributeWrite(buffer_ptr, buffer_len, UNIFORM_LIGHT_POSITION, pos_len);
        shader.set(str_buffer, m_pos);
    }

    return true; //TODO
}

Drawing::SpotLight::SpotLight(const LightProps& props, glm::vec3 dir, glm::vec3 pos, float cutoff_angle)
                                : Light(props), m_dir(glm::normalize(dir)),
                                  m_pos(pos), m_cos_cutoff_angle(cos(cutoff_angle)) {}

void Drawing::clear(GLFWwindow* window, Color color)
{
    const ColorF colorf = color.toFloat();

    glClearColor(colorf.r, colorf.g, colorf.b, colorf.a);
    glClear(GL_COLOR_BUFFER_BIT);
}
