#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "game.hpp"

#include "glm/ext/matrix_transform.hpp" //glm::lookAt
#include "glm/gtc/matrix_transform.hpp" //glm::perspective


Drawing::Camera3D::Camera3D(float fov, float aspect_ration, glm::vec3 pos, glm::vec3 target,
                            float near_plane, float far_plane)
                     : m_pos(pos), m_target(target), m_proj_mat()
{
    m_proj_mat = glm::perspective(glm::radians(fov), aspect_ration, near_plane, far_plane);
}

glm::mat4 Drawing::Camera3D::getViewMatrix() const
{
    //IDEA view matrix caching?
    return glm::lookAt(m_pos, m_target, Drawing::up_dir);
}

glm::mat4 Drawing::Camera3D::getProjectionMatrix() const
{
    //IDEA maybe pass this as const reference?
    return m_proj_mat;
}

void Drawing::Camera3D::setPosition(glm::vec3 pos)
{
    //IDEA view matrix caching?
    m_pos = pos;
}

void Drawing::clear(GLFWwindow* window, Color color)
{
    const ColorF colorf = color.toFloat();

    glClearColor(colorf.r, colorf.g, colorf.b, colorf.a);
    glClear(GL_COLOR_BUFFER_BIT);
}
