#include "AircraftGenerator.h"
#include <ctime>

AircraftGenerator::AircraftGenerator()
    : level_(1)
    , spawnTimer_(0.0f)
    , nextSpawnTime_(5.0f)
    , rng_(std::random_device{}())
{
}

AircraftGenerator::~AircraftGenerator() {}

void AircraftGenerator::init(int level)
{
    level_ = level;
    spawnTimer_ = 0.0f;
    scheduleNextSpawn();
}

void AircraftGenerator::reset()
{
    spawnTimer_ = 0.0f;
    scheduleNextSpawn();
}

void AircraftGenerator::setLevel(int level)
{
    level_ = level;
}

Aircraft* AircraftGenerator::trySpawn(float dt, int currentCount)
{
    auto& config = GameConfig::getInstance();
    config.setLevel(level_);

    if (currentCount >= config.getMaxConcurrentAircraft()) {
        return nullptr;
    }

    spawnTimer_ += dt;
    if (spawnTimer_ < nextSpawnTime_) {
        return nullptr;
    }

    // Time to spawn!
    spawnTimer_ = 0.0f;
    scheduleNextSpawn();

    AircraftType type = selectAircraftType();
    bool friendly = decideFriendly();

    // Override type for friendly aircraft
    if (friendly) {
        std::uniform_int_distribution<int> dist(0, 1);
        type = (dist(rng_) == 0) ? AircraftType::CIVILIAN_AIRLINER
                                 : AircraftType::FRIENDLY_MILITARY;
    }

    float startAzimuth = generateStartAzimuth();
    float startRange = generateStartRange();
    float altitude = generateAltitude(type);
    float speed = generateSpeed(type) * config.getSpeedMultiplier();
    float heading = generateHeading(startAzimuth);

    return new Aircraft(type, startRange, startAzimuth,
                        altitude, speed, heading, friendly);
}

AircraftType AircraftGenerator::selectAircraftType()
{
    auto& config = GameConfig::getInstance();

    // Weighted selection based on level
    struct TypeWeight {
        AircraftType type;
        int weight;
    };

    std::vector<TypeWeight> weights = {
        { AircraftType::RECON_DRONE,       30 },
        { AircraftType::ATTACK_DRONE,      25 },
        { AircraftType::FIGHTER_ATTACK,    20 },
        { AircraftType::TACTICAL_BOMBER,   15 },
        { AircraftType::STRATEGIC_BOMBER,  10 },
    };

    if (config.isStealthEnabled()) {
        weights.push_back({ AircraftType::STEALTH_FIGHTER, 8 });
    }

    int totalWeight = 0;
    for (const auto& w : weights) totalWeight += w.weight;

    std::uniform_int_distribution<int> dist(0, totalWeight - 1);
    int roll = dist(rng_);

    int cumulative = 0;
    for (const auto& w : weights) {
        cumulative += w.weight;
        if (roll < cumulative) return w.type;
    }

    return AircraftType::FIGHTER_ATTACK;
}

bool AircraftGenerator::decideFriendly()
{
    auto& config = GameConfig::getInstance();
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(rng_) < config.getFriendlyRatio();
}

float AircraftGenerator::randomRange(float min, float max)
{
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng_);
}

float AircraftGenerator::generateStartAzimuth()
{
    return randomRange(0.0f, 360.0f);
}

float AircraftGenerator::generateStartRange()
{
    // Aircraft appear at edge of radar range (85-100% of max range)
    float maxRange = GameConstants::RADAR_MAX_RANGE_KM;
    return randomRange(maxRange * 0.85f, maxRange);
}

float AircraftGenerator::generateAltitude(AircraftType type)
{
    switch (type) {
        case AircraftType::STRATEGIC_BOMBER:
            return randomRange(25000.0f, 45000.0f);
        case AircraftType::FIGHTER_ATTACK:
            return randomRange(5000.0f, 35000.0f);
        case AircraftType::TACTICAL_BOMBER:
            return randomRange(15000.0f, 30000.0f);
        case AircraftType::RECON_DRONE:
            return randomRange(10000.0f, 50000.0f);
        case AircraftType::ATTACK_DRONE:
            return randomRange(500.0f, 15000.0f);
        case AircraftType::STEALTH_FIGHTER:
            return randomRange(15000.0f, 40000.0f);
        case AircraftType::CIVILIAN_AIRLINER:
            return randomRange(28000.0f, 42000.0f);
        case AircraftType::FRIENDLY_MILITARY:
            return randomRange(10000.0f, 35000.0f);
    }
    return 20000.0f;
}

float AircraftGenerator::generateSpeed(AircraftType type)
{
    switch (type) {
        case AircraftType::STRATEGIC_BOMBER:
            return randomRange(400.0f, 600.0f);
        case AircraftType::FIGHTER_ATTACK:
            return randomRange(500.0f, 900.0f);
        case AircraftType::TACTICAL_BOMBER:
            return randomRange(350.0f, 500.0f);
        case AircraftType::RECON_DRONE:
            return randomRange(100.0f, 200.0f);
        case AircraftType::ATTACK_DRONE:
            return randomRange(150.0f, 300.0f);
        case AircraftType::STEALTH_FIGHTER:
            return randomRange(500.0f, 800.0f);
        case AircraftType::CIVILIAN_AIRLINER:
            return randomRange(400.0f, 500.0f);
        case AircraftType::FRIENDLY_MILITARY:
            return randomRange(350.0f, 600.0f);
    }
    return 400.0f;
}

float AircraftGenerator::generateHeading(float startAzimuth)
{
    // Aircraft should generally head toward the center (territory)
    // with some variation
    float towardCenter = startAzimuth + 180.0f;
    if (towardCenter >= 360.0f) towardCenter -= 360.0f;

    // Add some random deviation (+/- 45 degrees)
    float deviation = randomRange(-45.0f, 45.0f);
    float heading = towardCenter + deviation;
    if (heading < 0) heading += 360.0f;
    if (heading >= 360.0f) heading -= 360.0f;

    return heading;
}

void AircraftGenerator::scheduleNextSpawn()
{
    nextSpawnTime_ = GameConfig::getInstance().getSpawnInterval();
}
