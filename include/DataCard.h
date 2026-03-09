#ifndef __DATA_CARD_H__
#define __DATA_CARD_H__

#include "GameTypes.h"
#include <string>
#include <vector>
#include <cstdio>

// ============================================================================
// Data Cards — equipment specifications, troop strength, and battery intel
//
// These are the reference cards available to the AN/TSQ-73 Missile Minder
// operator, providing tactical data about weapon systems, personnel, and
// current operational status of each battery/platoon.
// ============================================================================

struct EquipmentCard {
    std::string systemDesignation;   // e.g., "MIM-104 PATRIOT"
    std::string radarSystem;         // e.g., "AN/MPQ-53"
    std::string engagementStation;   // e.g., "AN/MSQ-104 ECS"
    std::string launcher;            // e.g., "M901 Launching Station"
    std::string missileType;         // e.g., "MIM-104C PAC-2 GEM"
    float maxRangeKm;
    float maxRangeNM;
    float minRangeKm;
    float maxAltFt;
    float minAltFt;
    float missileSpeedMs;
    float missileSpeedMach;
    int launcherCapacity;
    std::string guidanceType;        // e.g., "Track-Via-Missile (TVM)"
    std::string propulsion;          // e.g., "Solid-fuel rocket"
    float baseKillProb;

    std::string format() const {
        char buf[1024];
        snprintf(buf, sizeof(buf),
            "=== EQUIPMENT DATA CARD ===\n"
            "SYSTEM:    %s\n"
            "RADAR:     %s\n"
            "ECS:       %s\n"
            "LAUNCHER:  %s\n"
            "MISSILE:   %s\n"
            "GUIDANCE:  %s\n"
            "PROPUL:    %s\n"
            "RNG MAX:   %.0f km (%.0f NM)\n"
            "RNG MIN:   %.1f km\n"
            "ALT MAX:   %.0f ft\n"
            "ALT MIN:   %.0f ft\n"
            "MSL SPD:   %.0f m/s (Mach %.1f)\n"
            "CAPACITY:  %d ready\n"
            "Pk BASE:   %.0f%%\n",
            systemDesignation.c_str(),
            radarSystem.c_str(),
            engagementStation.c_str(),
            launcher.c_str(),
            missileType.c_str(),
            guidanceType.c_str(),
            propulsion.c_str(),
            maxRangeKm, maxRangeNM,
            minRangeKm,
            maxAltFt,
            minAltFt,
            missileSpeedMs, missileSpeedMach,
            launcherCapacity,
            baseKillProb * 100.0f);
        return std::string(buf);
    }
};

struct TroopStrengthCard {
    std::string unitDesignation;     // e.g., "PATRIOT-1"
    std::string unitType;            // e.g., "Patriot Fire Unit"
    int personnelCount;              // Number of soldiers
    int officerCount;
    int enlistedCount;
    int operatorCount;               // Trained operators
    int loaderCount;                 // Loader crews
    int maintenanceCount;            // Maintenance personnel
    std::string readinessLevel;      // e.g., "C1 - FULLY READY"

    std::string format() const {
        char buf[512];
        snprintf(buf, sizeof(buf),
            "=== TROOP STRENGTH CARD ===\n"
            "UNIT:      %s\n"
            "TYPE:      %s\n"
            "STRENGTH:  %d total\n"
            "  Officers:    %d\n"
            "  Enlisted:    %d\n"
            "  Operators:   %d\n"
            "  Loaders:     %d\n"
            "  Maint:       %d\n"
            "READINESS: %s\n",
            unitDesignation.c_str(),
            unitType.c_str(),
            personnelCount,
            officerCount, enlistedCount,
            operatorCount, loaderCount, maintenanceCount,
            readinessLevel.c_str());
        return std::string(buf);
    }
};

struct BatteryIntelCard {
    std::string designation;
    PolarCoord currentPosition;
    bool isRelocating;
    float relocateTimeRemaining;     // Seconds remaining in relocation
    PolarCoord relocateDestination;  // Where battery is moving to
    int missilesReady;
    int missilesInStock;
    std::string currentStatus;
    std::string lastEngagement;      // e.g., "SPLASH TK-003 @ T+120s"
    int engagementsTotal;
    int hits;
    int misses;

    std::string format() const {
        char buf[512];
        float hitRate = (engagementsTotal > 0)
            ? (float)hits / engagementsTotal * 100.0f : 0.0f;
        snprintf(buf, sizeof(buf),
            "=== BATTERY INTEL CARD ===\n"
            "DESIG:     %s\n"
            "POS:       %.1f km / %03d deg\n"
            "STATUS:    %s%s\n"
            "MISSILES:  %d ready / %d stock\n"
            "ENGAGE:    %d total (%d hit, %d miss)\n"
            "HIT RATE:  %.0f%%\n"
            "LAST:      %s\n",
            designation.c_str(),
            currentPosition.range, (int)currentPosition.azimuth,
            currentStatus.c_str(),
            isRelocating ? " [RELOCATING]" : "",
            missilesReady, missilesInStock,
            engagementsTotal, hits, misses,
            hitRate,
            lastEngagement.empty() ? "N/A" : lastEngagement.c_str());
        return std::string(buf);
    }
};

// ============================================================================
// DataCardManager — provides access to all data cards
// ============================================================================

class DataCardManager {
public:
    // Get equipment card for a battery type
    static EquipmentCard getEquipmentCard(BatteryType type);

    // Get troop strength card for a specific battery
    static TroopStrengthCard getTroopStrengthCard(BatteryType type,
                                                   const std::string& designation);

    // Get all equipment cards
    static std::vector<EquipmentCard> getAllEquipmentCards();
};

#endif // __DATA_CARD_H__
