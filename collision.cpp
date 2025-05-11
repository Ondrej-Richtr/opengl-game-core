#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "game.hpp"

#include "glm/gtx/norm.hpp" //glm::length2


Collision::Ray::Ray(glm::vec3 pos, glm::vec3 dir)
                    : m_pos(pos), m_dir(dir) {}

Collision::RayCollision::RayCollision(float travel, glm::vec3 point)
                                        : m_hit(true), m_travel(travel), m_point(point)
{
    assert(travel >= 0.f);
}

glm::vec3 Collision::closestRayProjection(Collision::Ray ray, glm::vec3 point)
{
    // returns a point on the ray that is closest to given point
    assert(FLOAT_EQUALS(glm::length2(ray.m_dir), 1.f));

    const glm::vec3 point_rel = point - ray.m_pos; // point coords realtive to ray position
    float t = glm::dot(ray.m_dir, point_rel);

    if (t < 0.f) t = 0.f; // if the point is "behind" the ray then the closest point is the ray's position

    return ray.m_pos + t * ray.m_dir;
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

Collision::RayCollision Collision::raySphere(Collision::Ray ray, glm::vec3 sphere_pos, float sphere_radius)
{
    assert(sphere_radius >= 0.f || CLOSE_TO_0(sphere_radius));

    const glm::vec3 closest_point = closestRayProjection(ray, sphere_pos);
    float dist2 = glm::length2(sphere_pos - closest_point);

    if (dist2 <= sphere_radius * sphere_radius)
    {
        // closest point on the ray is inside the sphere -> ray must be passing through the sphere
        return Collision::RayCollision(glm::distance(closest_point, ray.m_pos), closest_point);
    }

    // else there can't be a point on the ray that is also inside the sphere -> no collision
    return {};
}

Collision::RayCollision Collision::rayFlatTargets(Ray ray, const std::vector<Game::Target>& targets,
                                                  double frame_time, size_t *out_idx)
{
    //IDEA we could check if the `target_normal` and `ray.m_dir` have positive or negative dot
    // and switch between front/back iteration based on that (however player should be always in the front anyways)
    const glm::vec3 target_normal = glm::vec3(0.f, 0.f, 1.f);

    for (size_t i = 0; i < targets.size(); ++i)
    {
        // iterating from the back, this ensures that the first hit should be the closest one
        size_t idx = targets.size() - 1 - i;

        const glm::vec3 pos_offset = glm::vec3(0.f, 0.f, FLOAT_TOLERANCE);
        const glm::vec3 pos = targets[idx].m_pos + pos_offset;
        const float radius = targets[idx].getScale(frame_time) * Game::Target::flat_target_size / 2.f;

        Collision::RayCollision rcoll = Collision::rayTarget(ray, target_normal, pos, radius);
        if (rcoll.m_hit)
        {
            if (out_idx) *out_idx = idx;
            return rcoll;
        }
    }

    return {}; // otherwise return empty collision
}

Collision::RayCollision Collision::rayBallTargets(Collision::Ray ray, const std::vector<Game::Target>& ball_targets,
                                                  double frame_time, size_t *out_idx)
{
    size_t closest_idx = 0;
    Collision::RayCollision closest_rcoll{};

    for (size_t idx = 0; idx < ball_targets.size(); ++idx)
    {
        const float radius = ball_targets[idx].getScale(frame_time) * Game::Target::ball_target_size / 2.f;

        Collision::RayCollision rcoll = Collision::raySphere(ray, ball_targets[idx].m_pos, radius);
        if (rcoll.m_hit && (!closest_rcoll.m_hit || rcoll.m_travel < closest_rcoll.m_travel))
        {
            // rcoll is closer hit than closest_hit or closest_hit is empty -> set rcoll as closest
            closest_idx = idx;
            closest_rcoll = rcoll;
        }
    }

    if (closest_rcoll.m_hit && out_idx) *out_idx = closest_idx; // set output index only when hit
    return closest_rcoll;
}
