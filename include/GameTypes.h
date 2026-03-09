#ifndef __GAME_TYPES_H__
#define __GAME_TYPES_H__

#include <string>
#include <cmath>

// ============================================================================
// Core Enumerations
// ============================================================================

enum class AircraftType {
    STRATEGIC_BOMBER,
    FIGHTER_ATTACK,
    TACTICAL_BOMBER,
    RECON_DRONE,
    ATTACK_DRONE,
    STEALTH_FIGHTER,
    CIVILIAN_AIRLINER,
    FRIENDLY_MILITARY
};

enum class IFFStatus {
    PENDING,      // Not yet interrogated
    FRIENDLY,     // Mode 4 positive response
    HOSTILE,      // No IFF response
    UNKNOWN       // Inconclusive / jammed
};

enum class TrackClassification {
    PENDING,
    FRIENDLY,
    HOSTILE,
    UNKNOWN
};

enum class BatteryType {
    PATRIOT,    // Mobile Patriot Missile Battery (MPMB)
    HAWK,       // Hawk Surface-to-Air Missile Battery (HSAMB)
    JAVELIN     // Javelin MANPADS Platoon (FGM-148, shoulder-launched)
};

enum class BatteryStatus {
    READY,          // Ready to fire
    TRACKING,       // Tracking a target
    ENGAGED,        // Missile in flight
    RELOADING,      // Reloading missiles
    DESTROYED,      // Battery destroyed
    OFFLINE         // Maintenance / offline
};

enum class EngagementResult {
    HIT,            // Target destroyed
    MISS,           // Missile missed
    IN_FLIGHT,      // Missile still in flight
    ABORTED         // Engagement aborted
};

// ============================================================================
// Data Structures
// ============================================================================

struct PolarCoord {
    float range;      // Distance in km
    float azimuth;    // Bearing in degrees (0-360, 0=North, clockwise)

    // Convert to screen-relative cartesian (x=east, y=north)
    float toScreenX(float scale) const {
        return range * scale * std::sin(azimuth * M_PI / 180.0f);
    }
    float toScreenY(float scale) const {
        return range * scale * std::cos(azimuth * M_PI / 180.0f);
    }
};

struct TrackData {
    int trackId;                        // e.g., 1 -> "TK-001"
    float altitude;                     // Feet MSL
    float azimuth;                      // Degrees (0-360)
    float range;                        // km from radar
    float speed;                        // Knots
    float heading;                      // Degrees (direction of travel)
    IFFStatus iffStatus;
    TrackClassification classification;
    AircraftType aircraftType;          // Actual type (may not be known to player)
    bool isAlive;
    float timeSinceLastSweep;           // For blip fade effect

    std::string getTrackIdString() const {
        char buf[16];
        snprintf(buf, sizeof(buf), "TK-%03d", trackId);
        return std::string(buf);
    }

    std::string getClassificationString() const {
        switch (classification) {
            case TrackClassification::FRIENDLY: return "FRIENDLY";
            case TrackClassification::HOSTILE:  return "HOSTILE";
            case TrackClassification::UNKNOWN:  return "UNKNOWN";
            case TrackClassification::PENDING:  return "PENDING";
        }
        return "---";
    }

    std::string getAltitudeString() const {
        char buf[16];
        if (altitude >= 1000.0f) {
            snprintf(buf, sizeof(buf), "FL%03d", (int)(altitude / 100.0f));
        } else {
            snprintf(buf, sizeof(buf), "%dft", (int)altitude);
        }
        return std::string(buf);
    }
};

struct BatteryData {
    std::string designation;     // e.g., "PATRIOT-1"
    BatteryType type;
    BatteryStatus status;
    int missilesRemaining;       // Ready missiles on launcher
    int maxMissiles;             // Launcher capacity
    int totalMissileStock;       // Total missiles in battery stock (including ready)
    int loaderCount;             // Number of loaders for reload operations
    float reloadTimeRemaining;   // Seconds until reload complete
    float maxRange;              // km
    float minRange;              // km
    float maxAltitude;           // feet
    float minAltitude;           // feet
    int assignedTrackId;         // -1 if not tracking
    PolarCoord position;         // Battery position relative to radar

    // Tracking radar info
    std::string trackingRadarType;   // e.g., "AN/MPQ-53", "AN/MPQ-46 HPI", "CLU IR/FLIR"
    bool hasMissileTracking;         // Provides missile guidance tracking
    int maxSimultaneousEngagements;  // How many targets can be engaged at once
};

struct EngagementRecord {
    int trackId;
    std::string batteryDesignation;
    EngagementResult result;
    float missileFlightTime;
    float elapsedTime;
};

// ============================================================================
// Game Configuration Constants
// ============================================================================

namespace GameConstants {
    // Radar — Raytheon AN/TPS-43E long-range surveillance radar
    // Range: 250+ nautical miles (~463 km), well beyond earth's curvature
    // Uses pulse-Doppler and MTI for beyond-horizon detection
    constexpr float RADAR_MAX_RANGE_NM = 250.0f;       // Nautical miles
    constexpr float RADAR_MAX_RANGE_KM = 463.0f;       // 250 NM in km
    constexpr float RADAR_SWEEP_RATE_RPM = 6.0f;       // Rotations per minute
    constexpr float RADAR_SWEEP_RATE_DPS = RADAR_SWEEP_RATE_RPM * 6.0f; // Degrees/sec
    constexpr int   RADAR_RANGE_RINGS = 5;              // Number of range rings
    constexpr float BLIP_FADE_TIME = 10.0f;             // Seconds for blip to fade

    // IFF
    constexpr float IFF_INTERROGATION_TIME = 2.0f;      // Seconds to get IFF response

    // Unit conversions
    constexpr float NM_TO_KM = 1.852f;                  // 1 nautical mile = 1.852 km
    constexpr float KM_TO_NM = 1.0f / NM_TO_KM;

    // Patriot Battery (MPMB) — MIM-104
    constexpr float PATRIOT_MAX_RANGE = 160.0f;          // km (~86 NM)
    constexpr float PATRIOT_MIN_RANGE = 3.0f;            // km
    constexpr float PATRIOT_MAX_ALT = 80000.0f;          // feet
    constexpr float PATRIOT_MIN_ALT = 1000.0f;           // feet
    constexpr int   PATRIOT_MAX_MISSILES = 4;
    constexpr float PATRIOT_RELOAD_TIME = 15.0f;         // seconds
    constexpr float PATRIOT_MISSILE_SPEED = 1700.0f;     // m/s (Mach 5)
    constexpr float PATRIOT_KILL_PROB = 0.85f;           // Base kill probability

    // Hawk Battery (HSAMB) — MIM-23
    // AN/MPQ-46 High Power Illuminator for target tracking
    // CW illuminator provides semi-active radar homing guidance
    constexpr float HAWK_MAX_RANGE = 45.0f;              // km (~24 NM)
    constexpr float HAWK_MIN_RANGE = 1.0f;               // km
    constexpr float HAWK_MAX_ALT = 45000.0f;             // feet
    constexpr float HAWK_MIN_ALT = 100.0f;               // feet (treetop)
    constexpr int   HAWK_MAX_MISSILES = 3;               // Ready on launcher
    constexpr int   HAWK_TOTAL_STOCK = 33;               // Total missiles per battery
    constexpr int   HAWK_LOADERS = 3;                    // Loader crews per battery
    constexpr float HAWK_RELOAD_TIME = 10.0f;            // seconds per loader cycle
    constexpr float HAWK_MISSILE_SPEED = 900.0f;         // m/s (Mach 2.7)
    constexpr float HAWK_KILL_PROB = 0.75f;              // Base kill probability

    // Javelin MANPADS Platoon — FGM-148 (game-adapted for close-in air defense)
    // CLU (Command Launch Unit) with IR/FLIR seeker
    // Electronic comms link to AN/TSQ-73 for target cueing
    // Close-combat defense: engages hostiles that breach the 30-mile perimeter
    constexpr float JAVELIN_MAX_RANGE = 55.0f;           // km (~30 NM) close combat zone
    constexpr float JAVELIN_MIN_RANGE = 0.5f;            // km
    constexpr float JAVELIN_MAX_ALT = 15000.0f;          // feet — low to medium altitude
    constexpr float JAVELIN_MIN_ALT = 0.0f;              // feet (ground level)
    constexpr int   JAVELIN_MAX_MISSILES = 2;            // Ready per team (CLU + 1 spare)
    constexpr float JAVELIN_RELOAD_TIME = 20.0f;         // seconds (manual reload)
    constexpr float JAVELIN_MISSILE_SPEED = 300.0f;      // m/s (~Mach 0.9)
    constexpr float JAVELIN_KILL_PROB = 0.65f;           // Base kill probability (IR seeker)

    // Scoring
    constexpr int SCORE_HOSTILE_DESTROYED_BASE = 100;
    constexpr int SCORE_FRIENDLY_DESTROYED = -1000;
    constexpr int SCORE_HOSTILE_PENETRATED = -200;
    constexpr int SCORE_MISSILE_WASTED = -25;
    constexpr float SCORE_FIRST_SHOT_MULTIPLIER = 2.0f;

    // Territory — inner defense zone
    constexpr float TERRITORY_RADIUS_KM = 25.0f;        // ~13.5 NM
}

#endif // __GAME_TYPES_H__
