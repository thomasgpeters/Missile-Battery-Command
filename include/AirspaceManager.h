#ifndef __AIRSPACE_MANAGER_H__
#define __AIRSPACE_MANAGER_H__

#include "GameTypes.h"
#include "Aircraft.h"
#include <vector>
#include <string>

// ============================================================================
// Airspace Zone Types
// ============================================================================

enum class AirspaceZoneType {
    FREE_ZONE,          // Friendly aircraft transit zone — do not engage
    NO_FLY_ZONE,        // Any aircraft here is hostile — weapons free
    RETURN_CORRIDOR     // Return-from-battle corridor with alt/heading constraints
};

// ============================================================================
// AirspaceZone — defines a wedge-shaped sector on the AN/TSQ-73 scope
//
// Zones are defined in polar coordinates relative to the radar center:
//   - Azimuth range (startAzimuth to endAzimuth, clockwise)
//   - Range band (minRange to maxRange in km)
//   - Altitude band (minAltitude to maxAltitude in feet)
//   - Heading constraints for corridors (allowedHeadingMin/Max)
// ============================================================================

struct AirspaceZone {
    std::string designation;     // e.g., "RTB-ALPHA", "NFZ-EAST", "FZ-SOUTH"
    AirspaceZoneType type;

    // Sector geometry (wedge on PPI scope)
    float startAzimuth;          // Degrees (0-360), start of sector
    float endAzimuth;            // Degrees (0-360), end of sector (clockwise)
    float minRange;              // km from radar center
    float maxRange;              // km from radar center

    // Altitude constraints
    float minAltitude;           // feet MSL
    float maxAltitude;           // feet MSL

    // Heading constraints (for return corridors)
    float allowedHeadingMin;     // Degrees — aircraft heading must be within this band
    float allowedHeadingMax;     // Degrees — to be considered friendly transit
    bool hasHeadingConstraint;   // Whether heading is checked

    bool active;                 // Zone is currently active

    // Check if an aircraft's position falls within this zone
    bool containsPosition(float azimuth, float range, float altitude) const;

    // Check if an aircraft matches corridor constraints (position + heading)
    bool matchesCorridor(float azimuth, float range, float altitude, float heading) const;

    // Get display name for zone type
    std::string getTypeString() const;
};

// ============================================================================
// AirspaceManager — manages all airspace control zones
//
// Integrated with the AN/TSQ-73 Missile Minder console:
//   - Free zones protect friendly transit routes
//   - No-fly zones flag all contacts as hostile
//   - Return corridors validate heading + altitude for RTB friendlies
// ============================================================================

class AirspaceManager {
public:
    AirspaceManager();
    ~AirspaceManager();

    // Zone management
    void addZone(const AirspaceZone& zone);
    void removeZone(const std::string& designation);
    void activateZone(const std::string& designation);
    void deactivateZone(const std::string& designation);
    void clearAllZones();

    // Aircraft evaluation — check what zone an aircraft is in
    // Returns the zone type affecting this aircraft, or empty string if none
    AirspaceZoneType evaluateAircraft(const Aircraft* aircraft) const;

    // Check if aircraft is in a free zone (should not be engaged)
    bool isInFreeZone(const Aircraft* aircraft) const;

    // Check if aircraft is in a no-fly zone (weapons free)
    bool isInNoFlyZone(const Aircraft* aircraft) const;

    // Check if aircraft matches a return corridor (altitude + heading match)
    bool matchesReturnCorridor(const Aircraft* aircraft) const;

    // Get all zones for display
    const std::vector<AirspaceZone>& getZones() const { return zones_; }

    // Get active zones only
    std::vector<const AirspaceZone*> getActiveZones() const;

    // Get zone by designation
    AirspaceZone* getZone(const std::string& designation);
    const AirspaceZone* getZone(const std::string& designation) const;

    // Set up default airspace for game start
    void initDefaultAirspace();

    // Helper: normalize azimuth check (handles wrap-around at 360)
    static bool azimuthInSector(float azimuth, float start, float end);

private:
    std::vector<AirspaceZone> zones_;
};

#endif // __AIRSPACE_MANAGER_H__
