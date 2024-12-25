#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "game.hpp"


Game::Target::Target(const Meshes::VBO& vbo, const Textures::Texture2D& texture, const MaterialProps& material,
                     glm::vec3 pos, double spawn_time)
                        : m_vbo(vbo), m_texture(texture), m_material(material),
                          m_pos(pos), m_spawn_time(spawn_time) {}

glm::vec2 Game::Target::getSize(double time) const
{
    static const double grow_time = 5.f; // 5 seconds
    static const float size_min = 0.1f;
    static const float size_max = 1.5f;
    assert(size_min <= size_max);

    if (time <= m_spawn_time) return glm::vec2(size_min);
    
    if (time >= m_spawn_time + grow_time) return glm::vec2(size_max);

    // linear interpolation:
    double t = (time - m_spawn_time) / grow_time;
    return glm::vec2((float)(t * size_max + (1.f - t) * size_min));
}

void Game::Target::draw(const Shaders::Program& shader, const Drawing::Camera3D& camera,
                        const std::vector<std::reference_wrapper<const Drawing::Light>>& lights, double current_frame_time) const
{
    Drawing::target(shader, camera, lights, *this, current_frame_time);
}
