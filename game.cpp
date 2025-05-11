#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "game.hpp"

#include <algorithm>


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

Game::Target::Target(const Meshes::Model& model, glm::vec3 pos, double spawn_time)
                : m_model(model), m_pos(pos), m_spawn_time(spawn_time) {}

Game::Target::Target(const Target& other)
                : m_model(other.m_model), m_pos(other.m_pos), m_spawn_time(other.m_spawn_time) {}

Game::Target& Game::Target::operator=(const Game::Target& other)
{
    memcpy((void*)this, &other, sizeof(Game::Target));
    return *this;
}

float Game::Target::getScale(double time) const
{
    assert(size_min <= size_max);

    if (time <= m_spawn_time) return size_min;
    
    if (time >= m_spawn_time + grow_time) return size_max;

    // linear interpolation:
    double t = (time - m_spawn_time) / grow_time;
    return static_cast<float>(t * size_max + (1.f - t) * size_min);
}

void Game::Target::draw(Game::TargetType type, const Drawing::Camera3D& camera,
                        const std::vector<std::reference_wrapper<const Lighting::Light>>& lights,
                        double current_frame_time, glm::vec3 pos_offset) const
{
    const float scale = getScale(current_frame_time);

    switch (type)
    {
    case Game::TargetType::target:
        {
            m_model.draw(camera, lights, m_pos + pos_offset, glm::vec3(scale, scale, 1.f));
            return;
        }
    case Game::TargetType::ball:
        {
            m_model.draw(camera, lights, m_pos + pos_offset, glm::vec3(scale));
            return;
        }
    }
}

Game::LevelPart::LevelPart(TargetType type, unsigned int target_amount, float spawn_rate,
                           SpawnNextFnPtr *spawn_next_fn, Color3F color)
        : m_spawn_next_fn(spawn_next_fn), m_type(type), m_target_amount(target_amount),
          m_spawn_rate(spawn_rate), m_color(color)
{
    assert(m_target_amount > 0); // level part without any targets makes no sense
    assert(spawn_next_fn != NULL); // spawning function must be defined
}

glm::vec3 Game::LevelPart::spawnNext(Utils::RNG& width, Utils::RNG& height, glm::vec2 wall_size) const
{
    //TODO probably pass pointer to instance as well as index of spawned target
    return m_spawn_next_fn(width, height, wall_size);
}

unsigned int Game::Level::getCummulativeCountUptoIndex(unsigned int idx) const
{
    assert(idx < m_target_amount_cum.size());
    return idx == 0 ? 0 : m_target_amount_cum[idx - 1];
}

Game::Level::Level(std::vector<LevelPart>&& level_parts, bool immediate_spawn)
        : m_target_amount_cum(), m_level_parts(std::move(level_parts)), m_immediate_spawn(immediate_spawn)
{
    assert(!m_level_parts.empty()); // level without any parts makes no sense

    unsigned target_sum = 0;
    for (size_t i = 0; i < m_level_parts.size(); ++i)
    {
        target_sum += m_level_parts[i].m_target_amount;
        m_target_amount_cum.push_back(target_sum);
    }
    assert(m_target_amount_cum.size() == m_level_parts.size());
    assert(target_sum > 0); // level without any targets makes no sense
}

const Game::LevelPart& Game::Level::getPart(unsigned int idx) const
{
    return m_level_parts[idx];
}

unsigned int Game::Level::getPartIdxFromSpawnedTargets(unsigned int targets_spawned) const
{
    //TODO check this, it might also be slower than simple linear loop
    auto begin = m_target_amount_cum.begin();
    auto end = m_target_amount_cum.end();

    auto upper = std::upper_bound(begin, end, targets_spawned);
    assert(upper != m_target_amount_cum.end());
    return static_cast<unsigned int>(std::distance(begin, upper));
}

unsigned int Game::Level::getTargetAmount() const
{
    return m_target_amount_cum.back();
}

void Game::LevelManager::addLevel(Level&& level)
{
    m_levels.emplace_back(std::move(level));
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

const Game::LevelPart* Game::LevelManager::getCurrentLevelPart(unsigned int targets_alive) const
{
    const Game::Level& level = getCurrentLevel();
    const unsigned int targets_spawned = m_level_targets_hit + targets_alive, level_targets = level.getTargetAmount();
    assert(targets_spawned <= level_targets);

    if (targets_spawned >= level_targets)
    {
        // all targets from current level has been spawned -> no level part is active
        return NULL;
    }

    const unsigned int level_part_idx = level.getPartIdxFromSpawnedTargets(targets_spawned);
    return &level.getPart(level_part_idx);
}

unsigned int Game::LevelManager::getCurrentLevelTargetAmount() const
{
    return getCurrentLevel().getTargetAmount();
}

unsigned int Game::LevelManager::getLevelAmount() const
{
    return static_cast<unsigned int>(m_levels.size());
}

unsigned int Game::LevelManager::getPartialTargetAmount(unsigned int level_from, unsigned int level_amount) const
{
    unsigned int result = 0, level_count = getLevelAmount();
    for (unsigned int i = level_from; i < level_from + level_amount && i < level_count; ++i)
    {
        result += getLevel(i).getTargetAmount();
    }

    return result;
}

unsigned int Game::LevelManager::getWholeTargetAmount() const
{
    return getPartialTargetAmount(0, getLevelAmount());
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

unsigned int Game::LevelManager::targetSpawnAmount(double frame_time, unsigned int targets_alive)
{
    const Game::Level& level = getCurrentLevel();
    const unsigned int level_targets = level.getTargetAmount();
    unsigned int targets_spawned = m_level_targets_hit + targets_alive;
    assert(targets_spawned <= level_targets);

    if (targets_spawned >= level_targets)
    {
        // dont spawn anything if all targets from current level have been already spawned
        // -> waiting for player to hit all alive targets
        assert(targets_alive > 0);
        return 0;
    }

    const unsigned int level_part_idx = level.getPartIdxFromSpawnedTargets(targets_spawned); 
    const Game::LevelPart& level_part = level.getPart(level_part_idx);

    const double time_to_spawn = 1.f / static_cast<double>(level_part.m_spawn_rate);
    assert(frame_time >= m_last_spawn_time);
    const double time_elapsed = frame_time - m_last_spawn_time;

    unsigned int spawn_amount = static_cast<unsigned int>(time_elapsed / time_to_spawn);

    //immediate spawn if set in level (takes effect only when no other targets alive or set to be spawned)
    if (level.m_immediate_spawn && !spawn_amount && !targets_alive) spawn_amount = 1;
    
    //limit spawn amount to at most the targets left in the level
    const unsigned int targets_left = level_targets - m_level_targets_hit - targets_alive; 
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
