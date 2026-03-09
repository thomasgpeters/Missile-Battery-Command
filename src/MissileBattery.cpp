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
{
    position_.range = posRange;
    position_.azimuth = posAzimuth;

    if (type == BatteryType::PATRIOT) {
        maxMissiles_ = GameConstants::PATRIOT_MAX_MISSILES;
        missilesRemaining_ = maxMissiles_;
        reloadTime_ = GameConstants::PATRIOT_RELOAD_TIME;
        maxRange_ = GameConstants::PATRIOT_MAX_RANGE;
        minRange_ = GameConstants::PATRIOT_MIN_RANGE;
        maxAltitude_ = GameConstants::PATRIOT_MAX_ALT;
        minAltitude_ = GameConstants::PATRIOT_MIN_ALT;
        missileSpeed_ = GameConstants::PATRIOT_MISSILE_SPEED;
        baseKillProbability_ = GameConstants::PATRIOT_KILL_PROB;
    } else {
        maxMissiles_ = GameConstants::HAWK_MAX_MISSILES;
        missilesRemaining_ = maxMissiles_;
        reloadTime_ = GameConstants::HAWK_RELOAD_TIME;
        maxRange_ = GameConstants::HAWK_MAX_RANGE;
        minRange_ = GameConstants::HAWK_MIN_RANGE;
        maxAltitude_ = GameConstants::HAWK_MAX_ALT;
        minAltitude_ = GameConstants::HAWK_MIN_ALT;
        missileSpeed_ = GameConstants::HAWK_MISSILE_SPEED;
        baseKillProbability_ = GameConstants::HAWK_KILL_PROB;
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
                if (currentTarget_ && currentTarget_->isAlive() && dist(rng) < killProb) {
                    lastResult_ = EngagementResult::HIT;
                    currentTarget_->destroy();
                } else {
                    lastResult_ = EngagementResult::MISS;
                }

                hasResult_ = true;
                currentTarget_ = nullptr;
                assignedTrackId_ = -1;

                // Check if reload needed
                if (missilesRemaining_ <= 0) {
                    status_ = BatteryStatus::RELOADING;
                    reloadTimeRemaining_ = reloadTime_;
                    missilesRemaining_ = maxMissiles_;
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
    return data;
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
