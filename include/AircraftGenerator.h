#ifndef __AIRCRAFT_GENERATOR_H__
#define __AIRCRAFT_GENERATOR_H__

#include "Aircraft.h"
#include "GameConfig.h"
#include <vector>
#include <memory>
#include <random>

// ============================================================================
// Random aircraft generator - spawns aircraft based on game level
// ============================================================================

class AircraftGenerator {
public:
    AircraftGenerator();
    ~AircraftGenerator();

    // Initialize for a given difficulty level
    void init(int level);

    // Update spawn timer; returns newly spawned aircraft (if any)
    // currentCount = number of aircraft currently alive on the field
    Aircraft* trySpawn(float dt, int currentCount);

    // Reset for new game/level
    void reset();

    // Configuration
    void setLevel(int level);
    int getLevel() const { return level_; }

private:
    int level_;
    float spawnTimer_;
    float nextSpawnTime_;
    std::mt19937 rng_;

    // Generate random aircraft parameters based on type
    AircraftType selectAircraftType();
    float randomRange(float min, float max);
    float generateStartAzimuth();
    float generateStartRange();
    float generateAltitude(AircraftType type);
    float generateSpeed(AircraftType type);
    float generateHeading(float startAzimuth);
    bool decideFriendly();

    void scheduleNextSpawn();
};

#endif // __AIRCRAFT_GENERATOR_H__
