#include "AirspaceManager.h"
#include <algorithm>
#include <cmath>

// ============================================================================
// AirspaceZone methods
// ============================================================================

bool AirspaceZone::containsPosition(float azimuth, float range, float altitude) const
{
    if (!active) return false;

    // Range check
    if (range < minRange || range > maxRange) return false;

    // Altitude check
    if (altitude < minAltitude || altitude > maxAltitude) return false;

    // Azimuth check (handles wrap-around)
    return AirspaceManager::azimuthInSector(azimuth, startAzimuth, endAzimuth);
}

bool AirspaceZone::matchesCorridor(float azimuth, float range, float altitude, float heading) const
{
    if (!containsPosition(azimuth, range, altitude)) return false;

    if (!hasHeadingConstraint) return true;

    // Check heading within allowed band
    return AirspaceManager::azimuthInSector(heading, allowedHeadingMin, allowedHeadingMax);
}

std::string AirspaceZone::getTypeString() const
{
    switch (type) {
        case AirspaceZoneType::FREE_ZONE:      return "FREE";
        case AirspaceZoneType::NO_FLY_ZONE:    return "NFZ";
        case AirspaceZoneType::RETURN_CORRIDOR: return "RTB";
    }
    return "---";
}

// ============================================================================
// AirspaceManager
// ============================================================================

AirspaceManager::AirspaceManager() {}
AirspaceManager::~AirspaceManager() {}

void AirspaceManager::addZone(const AirspaceZone& zone)
{
    zones_.push_back(zone);
}

void AirspaceManager::removeZone(const std::string& designation)
{
    zones_.erase(
        std::remove_if(zones_.begin(), zones_.end(),
            [&designation](const AirspaceZone& z) {
                return z.designation == designation;
            }),
        zones_.end());
}

void AirspaceManager::activateZone(const std::string& designation)
{
    auto* zone = getZone(designation);
    if (zone) zone->active = true;
}

void AirspaceManager::deactivateZone(const std::string& designation)
{
    auto* zone = getZone(designation);
    if (zone) zone->active = false;
}

void AirspaceManager::clearAllZones()
{
    zones_.clear();
}

AirspaceZoneType AirspaceManager::evaluateAircraft(const Aircraft* aircraft) const
{
    if (!aircraft || !aircraft->isAlive()) return AirspaceZoneType::FREE_ZONE;

    float az = aircraft->getAzimuth();
    float rng = aircraft->getRange();
    float alt = aircraft->getAltitude();
    float hdg = aircraft->getHeading();

    // Priority: no-fly zones first (weapons free), then corridors, then free zones
    for (const auto& zone : zones_) {
        if (!zone.active) continue;

        if (zone.type == AirspaceZoneType::NO_FLY_ZONE &&
            zone.containsPosition(az, rng, alt)) {
            return AirspaceZoneType::NO_FLY_ZONE;
        }
    }

    for (const auto& zone : zones_) {
        if (!zone.active) continue;

        if (zone.type == AirspaceZoneType::RETURN_CORRIDOR &&
            zone.matchesCorridor(az, rng, alt, hdg)) {
            return AirspaceZoneType::RETURN_CORRIDOR;
        }
    }

    for (const auto& zone : zones_) {
        if (!zone.active) continue;

        if (zone.type == AirspaceZoneType::FREE_ZONE &&
            zone.containsPosition(az, rng, alt)) {
            return AirspaceZoneType::FREE_ZONE;
        }
    }

    // Not in any defined zone
    return AirspaceZoneType::FREE_ZONE;
}

bool AirspaceManager::isInFreeZone(const Aircraft* aircraft) const
{
    if (!aircraft) return false;
    float az = aircraft->getAzimuth();
    float rng = aircraft->getRange();
    float alt = aircraft->getAltitude();

    for (const auto& zone : zones_) {
        if (zone.active && zone.type == AirspaceZoneType::FREE_ZONE &&
            zone.containsPosition(az, rng, alt)) {
            return true;
        }
    }
    return false;
}

bool AirspaceManager::isInNoFlyZone(const Aircraft* aircraft) const
{
    if (!aircraft) return false;
    float az = aircraft->getAzimuth();
    float rng = aircraft->getRange();
    float alt = aircraft->getAltitude();

    for (const auto& zone : zones_) {
        if (zone.active && zone.type == AirspaceZoneType::NO_FLY_ZONE &&
            zone.containsPosition(az, rng, alt)) {
            return true;
        }
    }
    return false;
}

bool AirspaceManager::matchesReturnCorridor(const Aircraft* aircraft) const
{
    if (!aircraft) return false;
    float az = aircraft->getAzimuth();
    float rng = aircraft->getRange();
    float alt = aircraft->getAltitude();
    float hdg = aircraft->getHeading();

    for (const auto& zone : zones_) {
        if (zone.active && zone.type == AirspaceZoneType::RETURN_CORRIDOR &&
            zone.matchesCorridor(az, rng, alt, hdg)) {
            return true;
        }
    }
    return false;
}

std::vector<const AirspaceZone*> AirspaceManager::getActiveZones() const
{
    std::vector<const AirspaceZone*> result;
    for (const auto& zone : zones_) {
        if (zone.active) result.push_back(&zone);
    }
    return result;
}

AirspaceZone* AirspaceManager::getZone(const std::string& designation)
{
    for (auto& zone : zones_) {
        if (zone.designation == designation) return &zone;
    }
    return nullptr;
}

const AirspaceZone* AirspaceManager::getZone(const std::string& designation) const
{
    for (const auto& zone : zones_) {
        if (zone.designation == designation) return &zone;
    }
    return nullptr;
}

void AirspaceManager::initDefaultAirspace()
{
    clearAllZones();

    // RTB Corridor ALPHA — Southwest return lane for friendlies
    // Aircraft heading ~045 (northeast toward base), FL200-FL400, azimuth 210-240
    AirspaceZone rtbAlpha;
    rtbAlpha.designation = "RTB-ALPHA";
    rtbAlpha.type = AirspaceZoneType::RETURN_CORRIDOR;
    rtbAlpha.startAzimuth = 210.0f;
    rtbAlpha.endAzimuth = 240.0f;
    rtbAlpha.minRange = 30.0f;
    rtbAlpha.maxRange = 463.0f;
    rtbAlpha.minAltitude = 20000.0f;
    rtbAlpha.maxAltitude = 40000.0f;
    rtbAlpha.allowedHeadingMin = 025.0f;
    rtbAlpha.allowedHeadingMax = 065.0f;
    rtbAlpha.hasHeadingConstraint = true;
    rtbAlpha.active = true;
    addZone(rtbAlpha);

    // RTB Corridor BRAVO — Northwest return lane
    // Aircraft heading ~135 (southeast toward base), FL200-FL400, azimuth 300-330
    AirspaceZone rtbBravo;
    rtbBravo.designation = "RTB-BRAVO";
    rtbBravo.type = AirspaceZoneType::RETURN_CORRIDOR;
    rtbBravo.startAzimuth = 300.0f;
    rtbBravo.endAzimuth = 330.0f;
    rtbBravo.minRange = 30.0f;
    rtbBravo.maxRange = 463.0f;
    rtbBravo.minAltitude = 20000.0f;
    rtbBravo.maxAltitude = 40000.0f;
    rtbBravo.allowedHeadingMin = 115.0f;
    rtbBravo.allowedHeadingMax = 155.0f;
    rtbBravo.hasHeadingConstraint = true;
    rtbBravo.active = true;
    addZone(rtbBravo);

    // No-Fly Zone EAST — restricted zone, weapons free
    // Low altitude from the east, likely attack vector
    AirspaceZone nfzEast;
    nfzEast.designation = "NFZ-EAST";
    nfzEast.type = AirspaceZoneType::NO_FLY_ZONE;
    nfzEast.startAzimuth = 80.0f;
    nfzEast.endAzimuth = 100.0f;
    nfzEast.minRange = 0.0f;
    nfzEast.maxRange = 463.0f;
    nfzEast.minAltitude = 0.0f;
    nfzEast.maxAltitude = 10000.0f;
    nfzEast.allowedHeadingMin = 0.0f;
    nfzEast.allowedHeadingMax = 0.0f;
    nfzEast.hasHeadingConstraint = false;
    nfzEast.active = true;
    addZone(nfzEast);
}

bool AirspaceManager::azimuthInSector(float azimuth, float start, float end)
{
    // Normalize all to 0-360
    auto normalize = [](float a) -> float {
        while (a < 0.0f) a += 360.0f;
        while (a >= 360.0f) a -= 360.0f;
        return a;
    };

    azimuth = normalize(azimuth);
    start = normalize(start);
    end = normalize(end);

    if (start <= end) {
        // Simple case: no wrap-around (e.g., 80 to 100)
        return azimuth >= start && azimuth <= end;
    } else {
        // Wrap-around case (e.g., 350 to 10)
        return azimuth >= start || azimuth <= end;
    }
}
