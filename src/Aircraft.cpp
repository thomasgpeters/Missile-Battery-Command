#include "Aircraft.h"
#include <cmath>

Aircraft::Aircraft(AircraftType type, float startRange, float startAzimuth,
                   float altitude, float speed, float heading, bool isFriendly)
    : type_(type)
    , range_(startRange)
    , azimuth_(startAzimuth)
    , altitude_(altitude)
    , speed_(speed)
    , heading_(heading)
    , friendly_(isFriendly)
    , alive_(true)
    , reachedTerritory_(false)
    , trackId_(-1)
    , iffStatus_(IFFStatus::PENDING)
    , timeSinceLastSweep_(0.0f)
{
}

Aircraft::~Aircraft() {}

void Aircraft::update(float dt)
{
    if (!alive_) return;

    updatePosition(dt);
    checkTerritoryPenetration();
    timeSinceLastSweep_ += dt;
}

void Aircraft::updatePosition(float dt)
{
    // Convert speed from knots to km/s: 1 knot = 0.000514444 km/s
    float speedKmPerSec = speed_ * 0.000514444f;
    float distanceKm = speedKmPerSec * dt;

    // Aircraft heading is the direction it's flying (0=north, clockwise)
    // We need to calculate new range and azimuth from radar center
    float headingRad = heading_ * M_PI / 180.0f;
    float azimuthRad = azimuth_ * M_PI / 180.0f;

    // Current position in cartesian (relative to radar center)
    float currentX = range_ * std::sin(azimuthRad);
    float currentY = range_ * std::cos(azimuthRad);

    // Move along heading
    float dx = distanceKm * std::sin(headingRad);
    float dy = distanceKm * std::cos(headingRad);

    float newX = currentX + dx;
    float newY = currentY + dy;

    // Convert back to polar
    range_ = std::sqrt(newX * newX + newY * newY);
    azimuth_ = std::atan2(newX, newY) * 180.0f / M_PI;
    if (azimuth_ < 0) azimuth_ += 360.0f;
}

void Aircraft::checkTerritoryPenetration()
{
    if (range_ <= GameConstants::TERRITORY_RADIUS_KM && !friendly_) {
        reachedTerritory_ = true;
    }
}

float Aircraft::getRadarCrossSection() const
{
    switch (type_) {
        case AircraftType::STRATEGIC_BOMBER:  return 100.0f;   // Large
        case AircraftType::TACTICAL_BOMBER:   return 50.0f;    // Medium-Large
        case AircraftType::FIGHTER_ATTACK:    return 10.0f;    // Medium
        case AircraftType::RECON_DRONE:       return 1.0f;     // Small
        case AircraftType::ATTACK_DRONE:      return 2.0f;     // Small
        case AircraftType::STEALTH_FIGHTER:   return 0.1f;     // Very Small
        case AircraftType::CIVILIAN_AIRLINER: return 80.0f;    // Large
        case AircraftType::FRIENDLY_MILITARY: return 15.0f;    // Medium
    }
    return 10.0f;
}

void Aircraft::destroy()
{
    alive_ = false;
}

TrackData Aircraft::getTrackData() const
{
    TrackData data;
    data.trackId = trackId_;
    data.altitude = altitude_;
    data.azimuth = azimuth_;
    data.range = range_;
    data.speed = speed_;
    data.heading = heading_;
    data.iffStatus = iffStatus_;
    data.aircraftType = type_;
    data.isAlive = alive_;
    data.timeSinceLastSweep = timeSinceLastSweep_;

    // Derive classification from IFF status
    switch (iffStatus_) {
        case IFFStatus::FRIENDLY:
            data.classification = TrackClassification::FRIENDLY;
            break;
        case IFFStatus::HOSTILE:
            data.classification = TrackClassification::HOSTILE;
            break;
        case IFFStatus::UNKNOWN:
            data.classification = TrackClassification::UNKNOWN;
            break;
        case IFFStatus::PENDING:
            data.classification = TrackClassification::PENDING;
            break;
    }

    return data;
}

int Aircraft::getThreatScore() const
{
    if (friendly_) return 0;

    int score = 0;

    // Threat based on type
    switch (type_) {
        case AircraftType::STRATEGIC_BOMBER: score = 50; break;
        case AircraftType::FIGHTER_ATTACK:   score = 40; break;
        case AircraftType::STEALTH_FIGHTER:  score = 60; break;
        case AircraftType::TACTICAL_BOMBER:  score = 35; break;
        case AircraftType::ATTACK_DRONE:     score = 25; break;
        case AircraftType::RECON_DRONE:      score = 10; break;
        default: score = 0; break;
    }

    // Closer = more threatening
    if (range_ < 30.0f) score += 30;
    else if (range_ < 50.0f) score += 15;

    // Faster = more threatening
    if (speed_ > 600.0f) score += 15;

    return score;
}

std::string Aircraft::getTypeName() const
{
    switch (type_) {
        case AircraftType::STRATEGIC_BOMBER:  return "STRAT BOMBER";
        case AircraftType::FIGHTER_ATTACK:    return "FIGHTER/ATK";
        case AircraftType::TACTICAL_BOMBER:   return "TAC BOMBER";
        case AircraftType::RECON_DRONE:       return "RECON UAV";
        case AircraftType::ATTACK_DRONE:      return "ATTACK UAV";
        case AircraftType::STEALTH_FIGHTER:   return "STEALTH FTR";
        case AircraftType::CIVILIAN_AIRLINER: return "CIVILIAN";
        case AircraftType::FRIENDLY_MILITARY: return "FRIENDLY MIL";
    }
    return "UNKNOWN";
}
