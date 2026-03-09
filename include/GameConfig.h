#ifndef __GAME_CONFIG_H__
#define __GAME_CONFIG_H__

#include "GameTypes.h"

// ============================================================================
// Game difficulty level configuration
// ============================================================================

struct LevelConfig {
    int level;
    int maxConcurrentAircraft;
    float spawnIntervalMin;      // Minimum seconds between spawns
    float spawnIntervalMax;      // Maximum seconds between spawns
    bool stealthEnabled;
    float iffErrorRate;          // 0.0 = no errors, 1.0 = all errors
    float speedMultiplier;       // Aircraft speed multiplier
    float friendlyRatio;         // Ratio of friendly to total aircraft
    int hostileScoreBonus;       // Extra score per hostile at this level
};

class GameConfig {
public:
    static GameConfig& getInstance();

    void setLevel(int level);
    int getLevel() const;
    const LevelConfig& getLevelConfig() const;

    // Aircraft spawn configuration for current level
    int getMaxConcurrentAircraft() const;
    float getSpawnInterval() const;   // Returns random interval within range
    bool isStealthEnabled() const;
    float getIFFErrorRate() const;
    float getSpeedMultiplier() const;
    float getFriendlyRatio() const;

    // Score for current level
    int getHostileDestroyedScore(AircraftType type) const;

private:
    GameConfig();
    int currentLevel_;
    static const LevelConfig LEVEL_CONFIGS[];
    static const int MAX_LEVEL;
};

#endif // __GAME_CONFIG_H__
