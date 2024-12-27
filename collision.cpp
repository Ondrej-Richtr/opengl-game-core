#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "game.hpp"

#include "glm/gtx/norm.hpp"


Collision::Ray::Ray(glm::vec3 pos, glm::vec3 dir)
                    : m_pos(pos), m_dir(dir) {}

Collision::RayCollision::RayCollision(float travel, glm::vec3 point)
                                        : m_hit(true), m_travel(travel), m_point(point)
{
    assert(travel >= 0.f);
}

Collision::RayCollision Collision::rayPlane(Collision::Ray ray, glm::vec3 plane_normal, glm::vec3 plane_pos)
{
    // returns intersection between 
    float r_dir_dot = glm::dot(ray.m_dir, plane_normal),
          r_pos_dot = glm::dot(ray.m_pos - plane_pos, plane_normal); // - plane_pos to translate into relative coord system

    if (CLOSE_TO_0(r_pos_dot)) // position is orthogonal to normal -> it lies on the checked plane
    {
        return Collision::RayCollision(0.f, ray.m_pos);
    }

    if (CLOSE_TO_0(r_dir_dot)) // direction parallel to plane -> no chance to intersect
    {
        return {};
    }
    
    float t = -(r_pos_dot / r_dir_dot);
    assert(!CLOSE_TO_0(t));

    if (t < 0.f) // direction points away from the plane -> no intersection
    {
        return {};
    }

    return Collision::RayCollision(t, ray.m_pos + t * ray.m_dir);
}

Collision::RayCollision Collision::rayTarget(Collision::Ray ray, glm::vec3 target_normal,
                                             glm::vec3 target_pos, float target_radius)
{
    Collision::RayCollision plane_rcoll = rayPlane(ray, target_normal, target_pos);
    if (!plane_rcoll.m_hit) return plane_rcoll; // no hit on the target plane -> no collision with the target

    if (glm::length2(plane_rcoll.m_point - target_pos)
            <= target_radius * target_radius) return plane_rcoll; // target plane collision is also target collision
    
    return {}; // otherwise no collision
}
