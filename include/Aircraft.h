#ifndef __AIRCRAFT_H__
#define __AIRCRAFT_H__

#include "GameTypes.h"

#if USE_COCOS2DX
#include "cocos2d.h"
#endif

// ============================================================================
// Aircraft entity - represents a single aircraft on the radar
// ============================================================================

class Aircraft {
public:
    Aircraft(AircraftType type, float startRange, float startAzimuth,
             float altitude, float speed, float heading, bool isFriendly);
    ~Aircraft();

    // Update aircraft position based on elapsed time
    void update(float dt);

    // Getters
    AircraftType getType() const { return type_; }
    float getRange() const { return range_; }
    float getAzimuth() const { return azimuth_; }
    float getAltitude() const { return altitude_; }
    float getSpeed() const { return speed_; }
    float getHeading() const { return heading_; }
    bool isFriendly() const { return friendly_; }
    bool isAlive() const { return alive_; }
    bool hasReachedTerritory() const { return reachedTerritory_; }
    float getRadarCrossSection() const;  // RCS for detection probability

    int getTrackId() const { return trackId_; }
    void setTrackId(int id) { trackId_ = id; }

    IFFStatus getIFFStatus() const { return iffStatus_; }
    void setIFFStatus(IFFStatus status) { iffStatus_ = status; }

    float getTimeSinceLastSweep() const { return timeSinceLastSweep_; }
    void resetSweepTimer() { timeSinceLastSweep_ = 0.0f; }
    void updateSweepTimer(float dt) { timeSinceLastSweep_ += dt; }

    // Combat
    void destroy();
    bool isInRange(float maxRange) const { return range_ <= maxRange; }

    // Populate track data for display
    TrackData getTrackData() const;

    // Threat scoring for engagement priority
    int getThreatScore() const;

    // Get type name string
    std::string getTypeName() const;

private:
    AircraftType type_;
    float range_;           // km from radar center
    float azimuth_;         // degrees, 0=north, clockwise
    float altitude_;        // feet MSL
    float speed_;           // knots
    float heading_;         // degrees, direction of travel
    bool friendly_;
    bool alive_;
    bool reachedTerritory_;
    int trackId_;
    IFFStatus iffStatus_;
    float timeSinceLastSweep_;

    // Movement calculation
    void updatePosition(float dt);
    void checkTerritoryPenetration();
};

#endif // __AIRCRAFT_H__
