#include "DataCard.h"

EquipmentCard DataCardManager::getEquipmentCard(BatteryType type)
{
    EquipmentCard card;

    switch (type) {
        case BatteryType::PATRIOT:
            card.systemDesignation = "MIM-104 PATRIOT (PAC-2)";
            card.radarSystem = "AN/MPQ-53 Phased Array";
            card.engagementStation = "AN/MSQ-104 ECS";
            card.launcher = "M901 Launching Station";
            card.missileType = "MIM-104C PAC-2 GEM";
            card.maxRangeKm = GameConstants::PATRIOT_MAX_RANGE;
            card.maxRangeNM = card.maxRangeKm * GameConstants::KM_TO_NM;
            card.minRangeKm = GameConstants::PATRIOT_MIN_RANGE;
            card.maxAltFt = GameConstants::PATRIOT_MAX_ALT;
            card.minAltFt = GameConstants::PATRIOT_MIN_ALT;
            card.missileSpeedMs = GameConstants::PATRIOT_MISSILE_SPEED;
            card.missileSpeedMach = card.missileSpeedMs / 343.0f;
            card.launcherCapacity = GameConstants::PATRIOT_MAX_MISSILES;
            card.guidanceType = "Track-Via-Missile (TVM)";
            card.propulsion = "Solid-fuel rocket motor";
            card.baseKillProb = GameConstants::PATRIOT_KILL_PROB;
            break;

        case BatteryType::HAWK:
            card.systemDesignation = "MIM-23 HAWK (I-HAWK)";
            card.radarSystem = "AN/MPQ-46 HPI + AN/MPQ-48 CWAR";
            card.engagementStation = "AN/TSW-12 BCC";
            card.launcher = "M192 Triple Launcher";
            card.missileType = "MIM-23B I-HAWK";
            card.maxRangeKm = GameConstants::HAWK_MAX_RANGE;
            card.maxRangeNM = card.maxRangeKm * GameConstants::KM_TO_NM;
            card.minRangeKm = GameConstants::HAWK_MIN_RANGE;
            card.maxAltFt = GameConstants::HAWK_MAX_ALT;
            card.minAltFt = GameConstants::HAWK_MIN_ALT;
            card.missileSpeedMs = GameConstants::HAWK_MISSILE_SPEED;
            card.missileSpeedMach = card.missileSpeedMs / 343.0f;
            card.launcherCapacity = GameConstants::HAWK_MAX_MISSILES;
            card.guidanceType = "Semi-Active Radar Homing (SARH)";
            card.propulsion = "Dual-thrust solid-fuel motor";
            card.baseKillProb = GameConstants::HAWK_KILL_PROB;
            break;

        case BatteryType::JAVELIN:
            card.systemDesignation = "FGM-148 JAVELIN (MANPADS)";
            card.radarSystem = "CLU IR/FLIR Sight";
            card.engagementStation = "AN/TSQ-73 Datalink (remote cueing)";
            card.launcher = "CLU (Command Launch Unit)";
            card.missileType = "FGM-148 Javelin";
            card.maxRangeKm = GameConstants::JAVELIN_MAX_RANGE;
            card.maxRangeNM = card.maxRangeKm * GameConstants::KM_TO_NM;
            card.minRangeKm = GameConstants::JAVELIN_MIN_RANGE;
            card.maxAltFt = GameConstants::JAVELIN_MAX_ALT;
            card.minAltFt = GameConstants::JAVELIN_MIN_ALT;
            card.missileSpeedMs = GameConstants::JAVELIN_MISSILE_SPEED;
            card.missileSpeedMach = card.missileSpeedMs / 343.0f;
            card.launcherCapacity = GameConstants::JAVELIN_MAX_MISSILES;
            card.guidanceType = "Imaging IR (fire-and-forget)";
            card.propulsion = "Soft-launch + solid-fuel flight motor";
            card.baseKillProb = GameConstants::JAVELIN_KILL_PROB;
            break;
    }

    return card;
}

TroopStrengthCard DataCardManager::getTroopStrengthCard(BatteryType type,
                                                         const std::string& designation)
{
    TroopStrengthCard card;
    card.unitDesignation = designation;

    switch (type) {
        case BatteryType::PATRIOT:
            card.unitType = "Patriot Fire Unit (MPMB)";
            card.personnelCount = 90;
            card.officerCount = 5;
            card.enlistedCount = 85;
            card.operatorCount = 12;
            card.loaderCount = 4;
            card.maintenanceCount = 16;
            card.readinessLevel = "C1 - FULLY MISSION CAPABLE";
            break;

        case BatteryType::HAWK:
            card.unitType = "Hawk SAM Battery (HSAMB)";
            card.personnelCount = 75;
            card.officerCount = 4;
            card.enlistedCount = 71;
            card.operatorCount = 8;
            card.loaderCount = GameConstants::HAWK_LOADERS;
            card.maintenanceCount = 12;
            card.readinessLevel = "C1 - FULLY MISSION CAPABLE";
            break;

        case BatteryType::JAVELIN:
            card.unitType = "Javelin MANPADS Platoon";
            card.personnelCount = 18;
            card.officerCount = 1;
            card.enlistedCount = 17;
            card.operatorCount = 6;   // 3 teams x 2 (gunner + AG)
            card.loaderCount = 0;     // Self-loaded
            card.maintenanceCount = 2;
            card.readinessLevel = "C1 - FULLY MISSION CAPABLE";
            break;
    }

    return card;
}

std::vector<EquipmentCard> DataCardManager::getAllEquipmentCards()
{
    return {
        getEquipmentCard(BatteryType::PATRIOT),
        getEquipmentCard(BatteryType::HAWK),
        getEquipmentCard(BatteryType::JAVELIN)
    };
}
