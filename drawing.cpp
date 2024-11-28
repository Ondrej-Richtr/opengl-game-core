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

void Drawing::Camera3D::setPosition(glm::vec3 pos)
{
    //IDEA view matrix caching?
    m_pos = pos;
}

void Drawing::Camera3D::movePosition(glm::vec3 move_vec)
{
    setPosition(m_pos + move_vec);
}

void Drawing::Camera3D::setTarget(glm::vec3 target)
{
    //IDEA view matrix caching?
    m_target = target;
}

void Drawing::Camera3D::moveTarget(glm::vec3 move_vec)
{
    setTarget(m_target + move_vec);
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

glm::vec3 Drawing::Camera3D::dirCoordsViewToWorld(glm::vec3 dir) const
{
    //TODO
    glm::mat3 m(getViewMatrix()); //IDEA cache this too
    glm::vec3 dir_transformed = glm::inverse(m) * dir;

    return NORMALIZE_OR_0(dir_transformed);
}

void Drawing::clear(GLFWwindow* window, Color color)
{
    const ColorF colorf = color.toFloat();

    glClearColor(colorf.r, colorf.g, colorf.b, colorf.a);
    glClear(GL_COLOR_BUFFER_BIT);
}
