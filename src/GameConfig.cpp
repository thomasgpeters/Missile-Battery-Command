#include "GameConfig.h"
#include <algorithm>
#include <random>

// ============================================================================
// Level configurations
// ============================================================================

const LevelConfig GameConfig::LEVEL_CONFIGS[] = {
    // level, maxAircraft, spawnMin, spawnMax, stealth, iffErr, speedMult, friendlyRatio, scoreBonus
    { 1,  3,  8.0f, 15.0f, false, 0.00f, 0.7f, 0.30f,   0 },
    { 2,  5,  6.0f, 12.0f, false, 0.00f, 0.85f, 0.25f,  25 },
    { 3,  8,  4.0f,  9.0f, false, 0.05f, 1.0f, 0.20f,   50 },
    { 4, 10,  3.0f,  7.0f, true,  0.10f, 1.15f, 0.15f,  75 },
    { 5, 15,  2.0f,  5.0f, true,  0.20f, 1.3f, 0.10f,  150 },
};

const int GameConfig::MAX_LEVEL = 5;

GameConfig& GameConfig::getInstance()
{
    static GameConfig instance;
    return instance;
}

GameConfig::GameConfig()
    : currentLevel_(1)
{
}

void GameConfig::setLevel(int level)
{
    currentLevel_ = std::clamp(level, 1, MAX_LEVEL);
}

int GameConfig::getLevel() const
{
    return currentLevel_;
}

const LevelConfig& GameConfig::getLevelConfig() const
{
    return LEVEL_CONFIGS[currentLevel_ - 1];
}

int GameConfig::getMaxConcurrentAircraft() const
{
    return getLevelConfig().maxConcurrentAircraft;
}

float GameConfig::getSpawnInterval() const
{
    const auto& cfg = getLevelConfig();
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(cfg.spawnIntervalMin, cfg.spawnIntervalMax);
    return dist(rng);
}

bool GameConfig::isStealthEnabled() const
{
    return getLevelConfig().stealthEnabled;
}

float GameConfig::getIFFErrorRate() const
{
    return getLevelConfig().iffErrorRate;
}

float GameConfig::getSpeedMultiplier() const
{
    return getLevelConfig().speedMultiplier;
}

float GameConfig::getFriendlyRatio() const
{
    return getLevelConfig().friendlyRatio;
}

int GameConfig::getHostileDestroyedScore(AircraftType type) const
{
    int baseScore = GameConstants::SCORE_HOSTILE_DESTROYED_BASE;
    const auto& cfg = getLevelConfig();

    switch (type) {
        case AircraftType::STRATEGIC_BOMBER: baseScore = 500; break;
        case AircraftType::FIGHTER_ATTACK:   baseScore = 300; break;
        case AircraftType::TACTICAL_BOMBER:  baseScore = 250; break;
        case AircraftType::ATTACK_DRONE:     baseScore = 200; break;
        case AircraftType::RECON_DRONE:      baseScore = 100; break;
        case AircraftType::STEALTH_FIGHTER:  baseScore = 500; break;
        default: baseScore = 100; break;
    }

    return baseScore + cfg.hostileScoreBonus;
}
