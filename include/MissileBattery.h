#ifndef __MISSILE_BATTERY_H__
#define __MISSILE_BATTERY_H__

#include "GameTypes.h"
#include "Aircraft.h"
#include <string>

// ============================================================================
// Missile Battery - Patriot (MPMB) or Hawk (HSAMB) battery unit
// ============================================================================

class MissileBattery {
public:
    MissileBattery(const std::string& designation, BatteryType type,
                   float posRange, float posAzimuth);
    ~MissileBattery();

    // Update battery state (reload timers, missile flight, etc.)
    void update(float dt);

    // Engage a target - returns true if engagement initiated
    bool engage(Aircraft* target);

    // Abort current engagement
    void abortEngagement();

    // Check if battery can engage a given target
    bool canEngage(const Aircraft* target) const;

    // Get battery data for display
    BatteryData getData() const;

    // Getters
    const std::string& getDesignation() const { return designation_; }
    BatteryType getType() const { return type_; }
    BatteryStatus getStatus() const { return status_; }
    int getMissilesRemaining() const { return missilesRemaining_; }
    int getAssignedTrackId() const { return assignedTrackId_; }
    bool isReady() const { return status_ == BatteryStatus::READY; }

    // Check engagement result (call after missile flight time elapsed)
    EngagementResult getLastResult() const { return lastResult_; }
    bool hasEngagementResult() const { return hasResult_; }
    void clearEngagementResult() { hasResult_ = false; }

private:
    std::string designation_;
    BatteryType type_;
    BatteryStatus status_;
    int missilesRemaining_;
    int maxMissiles_;
    float reloadTimeRemaining_;
    float reloadTime_;
    float maxRange_;
    float minRange_;
    float maxAltitude_;
    float minAltitude_;
    float missileSpeed_;       // m/s
    float baseKillProbability_;
    PolarCoord position_;

    // Engagement state
    int assignedTrackId_;
    Aircraft* currentTarget_;
    float missileFlightTime_;
    float missileFlightElapsed_;
    EngagementResult lastResult_;
    bool hasResult_;

    // Calculate kill probability against specific target
    float calculateKillProbability(const Aircraft* target) const;
    float calculateFlightTime(const Aircraft* target) const;
};

#endif // __MISSILE_BATTERY_H__
