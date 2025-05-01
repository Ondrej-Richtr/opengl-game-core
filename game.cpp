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

Game::Level::Level(unsigned int target_amount, float spawn_rate, SpawnNextFnPtr *spawn_next_fn, bool immediate_spawn)
        : m_spawn_next_fn(spawn_next_fn), m_target_amount(target_amount), m_spawn_rate(spawn_rate),
          m_immediate_spawn(immediate_spawn)
{
    assert(target_amount > 0); // level without targets makes no sense
}

glm::vec3 Game::Level::spawnNext(Utils::RNG& width, Utils::RNG& height, glm::vec2 wall_size) const
{
    return m_spawn_next_fn(width, height, wall_size);
}

void Game::LevelManager::addLevel(Level&& level)
{
    m_levels.push_back(std::move(level));
}

const Game::Level& Game::LevelManager::getLevel(unsigned int idx) const
{
    assert(idx < m_levels.size());
    return m_levels[idx];
}

const Game::Level& Game::LevelManager::getCurrentLevel() const
{
    assert(m_level_idx < m_levels.size());
    return getLevel(m_level_idx);
}

unsigned int Game::LevelManager::getCurrentLevelTargetAmount() const
{
    return getCurrentLevel().m_target_amount;
}

void Game::LevelManager::prepareFirstLevel(double frame_time)
{
    m_last_spawn_time = frame_time;
    m_level_idx = 0;
    m_level_targets_hit = 0;
}

bool Game::LevelManager::handleTargetHit(double frame_time)
{
    ++m_level_targets_hit;
    unsigned int level_target_amount = getCurrentLevelTargetAmount();
    printf("Hit! Targets hit: %d/%d\n", m_level_targets_hit, level_target_amount);

    assert(m_level_targets_hit <= level_target_amount);
    if (m_level_targets_hit >= level_target_amount)
    {
        printf("Level %d completed!\n", m_level_idx);
        ++m_level_idx;
        m_level_targets_hit = 0;
        m_last_spawn_time = frame_time;
        
        return true;
    }

    return false;
}

unsigned int Game::LevelManager::targetSpawnAmount(double frame_time, unsigned int targets_spawned)
{
    const Game::Level& level = getCurrentLevel();
    const double time_to_spawn = 1.f / static_cast<double>(level.m_spawn_rate);
    assert(frame_time >= m_last_spawn_time);
    const double time_elapsed = frame_time - m_last_spawn_time;

    unsigned int spawn_amount = static_cast<unsigned int>(time_elapsed / time_to_spawn);

    //immediate spawn if set in level (takes effect only when no other targets spawned or set to be spawned)
    if (level.m_immediate_spawn && !spawn_amount && !targets_spawned) spawn_amount = 1;
    
    //limit spawn amount to at most the targets left in the level
    const unsigned int level_targets = getCurrentLevelTargetAmount();
    assert(m_level_targets_hit < level_targets); // there should be always some targets left in the level
    assert(m_level_targets_hit + targets_spawned <= level_targets);
    const unsigned int targets_left = level_targets - m_level_targets_hit - targets_spawned; 
    if (spawn_amount > targets_left) spawn_amount = targets_left;

    //update last spawn time
    //NOTE simple solution, produces slightly lower spawn rate, especially on low fps
    // better solution might be to store m_last_spawn_time += spawn_amount * time_to_spawn;
    // but then the name m_last_spawn_time is incorrect and it also might cause inconsistencies if we expect
    // m_last_spawn_time to be some of the previous frame_times
    if (spawn_amount) m_last_spawn_time = frame_time;

    return spawn_amount;
}

bool Game::LevelManager::levelsCompleted() const
{
    return static_cast<size_t>(m_level_idx) >= m_levels.size();
}

glm::vec3 Game::targetRandomWallPosition(Utils::RNG& width, Utils::RNG& height, glm::vec2 wall_size)
{
    assert(wall_size.x > 0.f);
    float wall_width_half = wall_size.x / 2.f;
    float width_rolled = width.generateFloatRange(-wall_width_half, wall_width_half);

    assert(wall_size.y > 0.f);
    float wall_height_half = wall_size.y / 2.f;
    float height_rolled = height.generateFloatRange(-wall_height_half, wall_height_half);

    return glm::vec3(width_rolled, height_rolled, 0.f);
}

glm::vec3 Game::targetMiddleWallPosition(Utils::RNG& width, Utils::RNG& height, glm::vec2 wall_size)
{
    return glm::vec3(0.f);
}
