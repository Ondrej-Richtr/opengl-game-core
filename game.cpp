#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "game.hpp"


Game::Target::Target(const Meshes::VBO& vbo, const Lighting::Material& material, glm::vec3 pos, double spawn_time)
                        : m_vbo(vbo), m_material(material), m_pos(pos), m_spawn_time(spawn_time) {}

Game::Target::Target(const Target& other)
                        : m_vbo(other.m_vbo), m_material(other.m_material),
                          m_pos(other.m_pos), m_spawn_time(other.m_spawn_time) {}

Game::Target& Game::Target::operator=(const Game::Target& other)
{
    memcpy((void*)this, &other, sizeof(Game::Target));
    return *this;
}

glm::vec3 Game::Target::generateXZPosition(Utils::RNG& width, Utils::RNG& height, glm::vec2 wall_size)
{
    assert(wall_size.x > 0.f);
    float wall_width_half = wall_size.x / 2.f;
    float width_rolled = width.generateFloatRange(-wall_width_half, wall_width_half);

    assert(wall_size.y > 0.f);
    float wall_height_half = wall_size.y / 2.f;
    float height_rolled = height.generateFloatRange(-wall_height_half, wall_height_half);

    return glm::vec3(width_rolled, height_rolled, 0.f);
}

glm::vec2 Game::Target::getSize(double time) const
{
    assert(size_min <= size_max);

    if (time <= m_spawn_time) return glm::vec2(size_min);
    
    if (time >= m_spawn_time + grow_time) return glm::vec2(size_max);

    // linear interpolation:
    double t = (time - m_spawn_time) / grow_time;
    return glm::vec2((float)(t * size_max + (1.f - t) * size_min));
}

void Game::Target::draw(const Shaders::Program& shader, const Drawing::Camera3D& camera,
                        const std::vector<std::reference_wrapper<const Lighting::Light>>& lights,
                        double current_frame_time, glm::vec3 pos_offset) const
{
    Drawing::target(shader, camera, lights, *this, current_frame_time, pos_offset);
}
