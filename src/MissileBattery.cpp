#include "MissileBattery.h"
#include <cmath>
#include <random>

MissileBattery::MissileBattery(const std::string& designation, BatteryType type,
                               float posRange, float posAzimuth)
    : designation_(designation)
    , type_(type)
    , status_(BatteryStatus::READY)
    , reloadTimeRemaining_(0.0f)
    , assignedTrackId_(-1)
    , currentTarget_(nullptr)
    , missileFlightTime_(0.0f)
    , missileFlightElapsed_(0.0f)
    , lastResult_(EngagementResult::ABORTED)
    , hasResult_(false)
    , isRelocating_(false)
    , relocateTimeRemaining_(0.0f)
    , engagementCount_(0)
    , hitCount_(0)
    , missCount_(0)
{
    relocateDestination_.range = 0.0f;
    relocateDestination_.azimuth = 0.0f;
    position_.range = posRange;
    position_.azimuth = posAzimuth;

    if (type == BatteryType::PATRIOT) {
        maxMissiles_ = GameConstants::PATRIOT_MAX_MISSILES;
        missilesRemaining_ = maxMissiles_;
        totalMissileStock_ = 0;              // Patriot: all missiles on launcher, no extra stock
        loaderCount_ = 1;                    // Single launcher reload
        reloadTime_ = GameConstants::PATRIOT_RELOAD_TIME;
        maxRange_ = GameConstants::PATRIOT_MAX_RANGE;
        minRange_ = GameConstants::PATRIOT_MIN_RANGE;
        maxAltitude_ = GameConstants::PATRIOT_MAX_ALT;
        minAltitude_ = GameConstants::PATRIOT_MIN_ALT;
        missileSpeed_ = GameConstants::PATRIOT_MISSILE_SPEED;
        baseKillProbability_ = GameConstants::PATRIOT_KILL_PROB;
        // AN/MPQ-53 phased array radar — target track + missile guidance
        trackingRadarType_ = "AN/MPQ-53";
        hasMissileTracking_ = true;
        maxSimultaneousEngagements_ = 1;  // Game simplification
    } else if (type == BatteryType::HAWK) {
        maxMissiles_ = GameConstants::HAWK_MAX_MISSILES;
        missilesRemaining_ = maxMissiles_;
        totalMissileStock_ = GameConstants::HAWK_TOTAL_STOCK - maxMissiles_; // 30 in stock
        loaderCount_ = GameConstants::HAWK_LOADERS;
        reloadTime_ = GameConstants::HAWK_RELOAD_TIME;
        maxRange_ = GameConstants::HAWK_MAX_RANGE;
        minRange_ = GameConstants::HAWK_MIN_RANGE;
        maxAltitude_ = GameConstants::HAWK_MAX_ALT;
        minAltitude_ = GameConstants::HAWK_MIN_ALT;
        missileSpeed_ = GameConstants::HAWK_MISSILE_SPEED;
        baseKillProbability_ = GameConstants::HAWK_KILL_PROB;
        // AN/MPQ-46 HPI for target tracking, CW illuminator for missile guidance
        trackingRadarType_ = "AN/MPQ-46 HPI";
        hasMissileTracking_ = true;
        maxSimultaneousEngagements_ = 1;
    } else {  // JAVELIN
        maxMissiles_ = GameConstants::JAVELIN_MAX_MISSILES;
        missilesRemaining_ = maxMissiles_;
        totalMissileStock_ = 0;              // Javelin: what you carry is what you have
        loaderCount_ = 1;                    // Manual team reload
        reloadTime_ = GameConstants::JAVELIN_RELOAD_TIME;
        maxRange_ = GameConstants::JAVELIN_MAX_RANGE;
        minRange_ = GameConstants::JAVELIN_MIN_RANGE;
        maxAltitude_ = GameConstants::JAVELIN_MAX_ALT;
        minAltitude_ = GameConstants::JAVELIN_MIN_ALT;
        missileSpeed_ = GameConstants::JAVELIN_MISSILE_SPEED;
        baseKillProbability_ = GameConstants::JAVELIN_KILL_PROB;
        // CLU with IR/FLIR seeker — no radar, fire-and-forget
        trackingRadarType_ = "CLU IR/FLIR";
        hasMissileTracking_ = false;  // Fire-and-forget IR guidance
        maxSimultaneousEngagements_ = 1;
    }
}

MissileBattery::~MissileBattery() {}

void MissileBattery::update(float dt)
{
    switch (status_) {
        case BatteryStatus::ENGAGED: {
            // Missile is in flight
            missileFlightElapsed_ += dt;
            if (missileFlightElapsed_ >= missileFlightTime_) {
                // Missile has arrived - determine hit/miss
                static std::mt19937 rng(std::random_device{}());
                std::uniform_real_distribution<float> dist(0.0f, 1.0f);

                float killProb = calculateKillProbability(currentTarget_);
                engagementCount_++;
                if (currentTarget_ && currentTarget_->isAlive() && dist(rng) < killProb) {
                    lastResult_ = EngagementResult::HIT;
                    currentTarget_->destroy();
                    hitCount_++;
                } else {
                    lastResult_ = EngagementResult::MISS;
                    missCount_++;
                }

                hasResult_ = true;
                currentTarget_ = nullptr;
                assignedTrackId_ = -1;

                // Check if reload needed
                if (missilesRemaining_ <= 0) {
                    // Calculate how many missiles remain in stock (not on launcher)
                    int stockRemaining = totalMissileStock_;
                    int toReload = std::min(maxMissiles_, stockRemaining);
                    if (toReload > 0) {
                        status_ = BatteryStatus::RELOADING;
                        reloadTimeRemaining_ = reloadTime_;
                        missilesRemaining_ = toReload;
                        totalMissileStock_ -= toReload;
                    } else {
                        // No missiles left in stock — battery is dry
                        status_ = BatteryStatus::OFFLINE;
                    }
                } else {
                    status_ = BatteryStatus::READY;
                }
            }
            break;
        }

        case BatteryStatus::RELOADING: {
            reloadTimeRemaining_ -= dt;
            if (reloadTimeRemaining_ <= 0.0f) {
                reloadTimeRemaining_ = 0.0f;
                status_ = BatteryStatus::READY;
            }
            break;
        }

        case BatteryStatus::OFFLINE: {
            // Handle relocation countdown
            if (isRelocating_) {
                relocateTimeRemaining_ -= dt;
                if (relocateTimeRemaining_ <= 0.0f) {
                    // Relocation complete — deploy at new position
                    position_ = relocateDestination_;
                    isRelocating_ = false;
                    relocateTimeRemaining_ = 0.0f;
                    status_ = BatteryStatus::READY;
                }
            }
            break;
        }

        default:
            break;
    }
}

bool MissileBattery::engage(Aircraft* target)
{
    if (!canEngage(target)) return false;
    if (status_ != BatteryStatus::READY && status_ != BatteryStatus::TRACKING) {
        return false;
    }

    currentTarget_ = target;
    assignedTrackId_ = target->getTrackId();
    missilesRemaining_--;
    missileFlightTime_ = calculateFlightTime(target);
    missileFlightElapsed_ = 0.0f;
    status_ = BatteryStatus::ENGAGED;
    hasResult_ = false;

    return true;
}

void MissileBattery::abortEngagement()
{
    if (status_ == BatteryStatus::TRACKING) {
        status_ = BatteryStatus::READY;
        currentTarget_ = nullptr;
        assignedTrackId_ = -1;
    }
    // Can't abort a missile already in flight
}

bool MissileBattery::canEngage(const Aircraft* target) const
{
    if (!target || !target->isAlive()) return false;
    if (status_ != BatteryStatus::READY && status_ != BatteryStatus::TRACKING) {
        return false;
    }
    if (missilesRemaining_ <= 0) return false;

    float range = target->getRange();
    float alt = target->getAltitude();

    return range >= minRange_ && range <= maxRange_ &&
           alt >= minAltitude_ && alt <= maxAltitude_;
}

BatteryData MissileBattery::getData() const
{
    BatteryData data;
    data.designation = designation_;
    data.type = type_;
    data.status = status_;
    data.missilesRemaining = missilesRemaining_;
    data.maxMissiles = maxMissiles_;
    data.reloadTimeRemaining = reloadTimeRemaining_;
    data.maxRange = maxRange_;
    data.minRange = minRange_;
    data.maxAltitude = maxAltitude_;
    data.minAltitude = minAltitude_;
    data.assignedTrackId = assignedTrackId_;
    data.position = position_;
    data.totalMissileStock = totalMissileStock_ + missilesRemaining_;
    data.loaderCount = loaderCount_;
    data.trackingRadarType = trackingRadarType_;
    data.hasMissileTracking = hasMissileTracking_;
    data.maxSimultaneousEngagements = maxSimultaneousEngagements_;
    return data;
}

bool MissileBattery::relocate(float newRange, float newAzimuth)
{
    // Cannot relocate while engaged or already relocating
    if (status_ == BatteryStatus::ENGAGED) return false;
    if (isRelocating_) return false;
    if (status_ == BatteryStatus::DESTROYED) return false;

    relocateDestination_.range = newRange;
    relocateDestination_.azimuth = newAzimuth;
    isRelocating_ = true;

    // Relocation time based on battery type:
    //   Patriot: ~60 min to emplace, simplified to 60 seconds game time
    //   Hawk:    ~45 min, simplified to 45 seconds
    //   Javelin: ~10 min (foot mobile), simplified to 15 seconds
    switch (type_) {
        case BatteryType::PATRIOT: relocateTimeRemaining_ = 60.0f; break;
        case BatteryType::HAWK:    relocateTimeRemaining_ = 45.0f; break;
        case BatteryType::JAVELIN: relocateTimeRemaining_ = 15.0f; break;
    }

    // Battery goes offline during move
    status_ = BatteryStatus::OFFLINE;
    currentTarget_ = nullptr;
    assignedTrackId_ = -1;

    return true;
}

float MissileBattery::calculateKillProbability(const Aircraft* target) const
{
    if (!target) return 0.0f;

    float prob = baseKillProbability_;

    // Stealth penalty
    if (target->getType() == AircraftType::STEALTH_FIGHTER) {
        prob *= 0.6f;
    }

    // Fast movers harder to hit
    if (target->getSpeed() > 700.0f) {
        prob *= 0.85f;
    }

    // Low altitude penalty for Patriot
    if (type_ == BatteryType::PATRIOT && target->getAltitude() < 5000.0f) {
        prob *= 0.7f;
    }

    // Drone bonus for Hawk (optimized for smaller targets)
    if (type_ == BatteryType::HAWK &&
        (target->getType() == AircraftType::ATTACK_DRONE ||
         target->getType() == AircraftType::RECON_DRONE)) {
        prob *= 1.1f;
    }

    // Javelin IR seeker bonuses/penalties
    if (type_ == BatteryType::JAVELIN) {
        // IR seeker is good against slow, hot targets (helicopters, drones)
        if (target->getType() == AircraftType::ATTACK_DRONE) {
            prob *= 1.2f;
        }
        // Fast movers are very hard for shoulder-launched
        if (target->getSpeed() > 500.0f) {
            prob *= 0.5f;
        }
        // Stealth doesn't help against IR
        if (target->getType() == AircraftType::STEALTH_FIGHTER) {
            prob *= 1.3f;  // Negate stealth penalty — IR doesn't care
        }
    }

    // Range penalty - farther = less accurate
    float rangeFraction = target->getRange() / maxRange_;
    if (rangeFraction > 0.8f) {
        prob *= 0.9f;
    }

    return std::min(prob, 0.95f);  // Cap at 95%
}

float MissileBattery::calculateFlightTime(const Aircraft* target) const
{
    if (!target) return 0.0f;

    // Simplified: distance in km / missile speed in km/s
    float distanceKm = target->getRange();
    float speedKmPerSec = missileSpeed_ / 1000.0f;

    return distanceKm / speedKmPerSec;
}
