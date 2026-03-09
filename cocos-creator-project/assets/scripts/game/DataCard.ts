import { BatteryType, GameConstants } from './GameTypes';

// ============================================================================
// Data Cards — equipment specifications, troop strength, and battery intel
// ============================================================================

export interface EquipmentCard {
    systemDesignation: string;
    radarSystem: string;
    engagementStation: string;
    launcher: string;
    missileType: string;
    maxRangeKm: number;
    maxRangeNM: number;
    minRangeKm: number;
    maxAltFt: number;
    minAltFt: number;
    missileSpeedMs: number;
    missileSpeedMach: number;
    launcherCapacity: number;
    guidanceType: string;
    propulsion: string;
    baseKillProb: number;
}

export interface TroopStrengthCard {
    unitDesignation: string;
    unitType: string;
    personnelCount: number;
    officerCount: number;
    enlistedCount: number;
    operatorCount: number;
    loaderCount: number;
    maintenanceCount: number;
    readinessLevel: string;
}

export interface BatteryIntelCard {
    designation: string;
    currentPosition: { range: number; azimuth: number };
    isRelocating: boolean;
    relocateTimeRemaining: number;
    relocateDestination: { range: number; azimuth: number };
    missilesReady: number;
    missilesInStock: number;
    currentStatus: string;
    lastEngagement: string;
    engagementsTotal: number;
    hits: number;
    misses: number;
}

export class DataCardManager {
    static getEquipmentCard(type: BatteryType): EquipmentCard {
        switch (type) {
            case BatteryType.PATRIOT:
                return {
                    systemDesignation: 'MIM-104 PATRIOT (PAC-2)',
                    radarSystem: 'AN/MPQ-53 Phased Array',
                    engagementStation: 'AN/MSQ-104 ECS',
                    launcher: 'M901 Launching Station',
                    missileType: 'MIM-104C PAC-2 GEM',
                    maxRangeKm: GameConstants.PATRIOT_MAX_RANGE,
                    maxRangeNM: GameConstants.PATRIOT_MAX_RANGE * GameConstants.KM_TO_NM,
                    minRangeKm: GameConstants.PATRIOT_MIN_RANGE,
                    maxAltFt: GameConstants.PATRIOT_MAX_ALT,
                    minAltFt: GameConstants.PATRIOT_MIN_ALT,
                    missileSpeedMs: GameConstants.PATRIOT_MISSILE_SPEED,
                    missileSpeedMach: GameConstants.PATRIOT_MISSILE_SPEED / 343.0,
                    launcherCapacity: GameConstants.PATRIOT_MAX_MISSILES,
                    guidanceType: 'Track-Via-Missile (TVM)',
                    propulsion: 'Solid-fuel rocket motor',
                    baseKillProb: GameConstants.PATRIOT_KILL_PROB,
                };
            case BatteryType.HAWK:
                return {
                    systemDesignation: 'MIM-23 HAWK (I-HAWK)',
                    radarSystem: 'AN/MPQ-46 HPI + AN/MPQ-48 CWAR',
                    engagementStation: 'AN/TSW-12 BCC',
                    launcher: 'M192 Triple Launcher',
                    missileType: 'MIM-23B I-HAWK',
                    maxRangeKm: GameConstants.HAWK_MAX_RANGE,
                    maxRangeNM: GameConstants.HAWK_MAX_RANGE * GameConstants.KM_TO_NM,
                    minRangeKm: GameConstants.HAWK_MIN_RANGE,
                    maxAltFt: GameConstants.HAWK_MAX_ALT,
                    minAltFt: GameConstants.HAWK_MIN_ALT,
                    missileSpeedMs: GameConstants.HAWK_MISSILE_SPEED,
                    missileSpeedMach: GameConstants.HAWK_MISSILE_SPEED / 343.0,
                    launcherCapacity: GameConstants.HAWK_MAX_MISSILES,
                    guidanceType: 'Semi-Active Radar Homing (SARH)',
                    propulsion: 'Dual-thrust solid-fuel motor',
                    baseKillProb: GameConstants.HAWK_KILL_PROB,
                };
            case BatteryType.JAVELIN:
                return {
                    systemDesignation: 'FGM-148 JAVELIN (MANPADS)',
                    radarSystem: 'CLU IR/FLIR Sight',
                    engagementStation: 'AN/TSQ-73 Datalink (remote cueing)',
                    launcher: 'CLU (Command Launch Unit)',
                    missileType: 'FGM-148 Javelin',
                    maxRangeKm: GameConstants.JAVELIN_MAX_RANGE,
                    maxRangeNM: GameConstants.JAVELIN_MAX_RANGE * GameConstants.KM_TO_NM,
                    minRangeKm: GameConstants.JAVELIN_MIN_RANGE,
                    maxAltFt: GameConstants.JAVELIN_MAX_ALT,
                    minAltFt: GameConstants.JAVELIN_MIN_ALT,
                    missileSpeedMs: GameConstants.JAVELIN_MISSILE_SPEED,
                    missileSpeedMach: GameConstants.JAVELIN_MISSILE_SPEED / 343.0,
                    launcherCapacity: GameConstants.JAVELIN_MAX_MISSILES,
                    guidanceType: 'Imaging IR (fire-and-forget)',
                    propulsion: 'Soft-launch + solid-fuel flight motor',
                    baseKillProb: GameConstants.JAVELIN_KILL_PROB,
                };
        }
    }

    static getTroopStrengthCard(type: BatteryType, designation: string): TroopStrengthCard {
        switch (type) {
            case BatteryType.PATRIOT:
                return {
                    unitDesignation: designation, unitType: 'Patriot Fire Unit (MPMB)',
                    personnelCount: 90, officerCount: 5, enlistedCount: 85,
                    operatorCount: 12, loaderCount: 4, maintenanceCount: 16,
                    readinessLevel: 'C1 - FULLY MISSION CAPABLE',
                };
            case BatteryType.HAWK:
                return {
                    unitDesignation: designation, unitType: 'Hawk SAM Battery (HSAMB)',
                    personnelCount: 75, officerCount: 4, enlistedCount: 71,
                    operatorCount: 8, loaderCount: GameConstants.HAWK_LOADERS, maintenanceCount: 12,
                    readinessLevel: 'C1 - FULLY MISSION CAPABLE',
                };
            case BatteryType.JAVELIN:
                return {
                    unitDesignation: designation, unitType: 'Javelin MANPADS Platoon',
                    personnelCount: 18, officerCount: 1, enlistedCount: 17,
                    operatorCount: 6, loaderCount: 0, maintenanceCount: 2,
                    readinessLevel: 'C1 - FULLY MISSION CAPABLE',
                };
        }
    }

    static getAllEquipmentCards(): EquipmentCard[] {
        return [
            DataCardManager.getEquipmentCard(BatteryType.PATRIOT),
            DataCardManager.getEquipmentCard(BatteryType.HAWK),
            DataCardManager.getEquipmentCard(BatteryType.JAVELIN),
        ];
    }
}
