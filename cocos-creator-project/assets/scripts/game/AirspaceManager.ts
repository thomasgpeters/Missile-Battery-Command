import { GameConstants } from './GameTypes';
import { Aircraft } from './Aircraft';

// ============================================================================
// Airspace Zone Types
// ============================================================================

export enum AirspaceZoneType {
    FREE_ZONE,
    NO_FLY_ZONE,
    RETURN_CORRIDOR,
}

export interface AirspaceZone {
    designation: string;
    type: AirspaceZoneType;
    startAzimuth: number;
    endAzimuth: number;
    minRange: number;
    maxRange: number;
    minAltitude: number;
    maxAltitude: number;
    allowedHeadingMin: number;
    allowedHeadingMax: number;
    hasHeadingConstraint: boolean;
    active: boolean;
}

function containsPosition(zone: AirspaceZone, azimuth: number, range: number, altitude: number): boolean {
    if (!zone.active) return false;
    if (range < zone.minRange || range > zone.maxRange) return false;
    if (altitude < zone.minAltitude || altitude > zone.maxAltitude) return false;
    return AirspaceManager.azimuthInSector(azimuth, zone.startAzimuth, zone.endAzimuth);
}

function matchesCorridor(zone: AirspaceZone, azimuth: number, range: number, altitude: number, heading: number): boolean {
    if (!containsPosition(zone, azimuth, range, altitude)) return false;
    if (!zone.hasHeadingConstraint) return true;
    return AirspaceManager.azimuthInSector(heading, zone.allowedHeadingMin, zone.allowedHeadingMax);
}

function getZoneTypeString(type: AirspaceZoneType): string {
    switch (type) {
        case AirspaceZoneType.FREE_ZONE:      return 'FREE';
        case AirspaceZoneType.NO_FLY_ZONE:    return 'NFZ';
        case AirspaceZoneType.RETURN_CORRIDOR: return 'RTB';
    }
}

// ============================================================================
// AirspaceManager — manages all airspace control zones
// ============================================================================

export class AirspaceManager {
    private zones: AirspaceZone[] = [];

    addZone(zone: AirspaceZone): void {
        this.zones.push(zone);
    }

    removeZone(designation: string): void {
        this.zones = this.zones.filter((z) => z.designation !== designation);
    }

    activateZone(designation: string): void {
        const zone = this.getZone(designation);
        if (zone) zone.active = true;
    }

    deactivateZone(designation: string): void {
        const zone = this.getZone(designation);
        if (zone) zone.active = false;
    }

    clearAllZones(): void {
        this.zones = [];
    }

    evaluateAircraft(aircraft: Aircraft | null): AirspaceZoneType {
        if (!aircraft || !aircraft.isAlive()) return AirspaceZoneType.FREE_ZONE;

        const az = aircraft.getAzimuth();
        const rng = aircraft.getRange();
        const alt = aircraft.getAltitude();
        const hdg = aircraft.getHeading();

        for (const zone of this.zones) {
            if (!zone.active) continue;
            if (zone.type === AirspaceZoneType.NO_FLY_ZONE &&
                containsPosition(zone, az, rng, alt)) {
                return AirspaceZoneType.NO_FLY_ZONE;
            }
        }

        for (const zone of this.zones) {
            if (!zone.active) continue;
            if (zone.type === AirspaceZoneType.RETURN_CORRIDOR &&
                matchesCorridor(zone, az, rng, alt, hdg)) {
                return AirspaceZoneType.RETURN_CORRIDOR;
            }
        }

        for (const zone of this.zones) {
            if (!zone.active) continue;
            if (zone.type === AirspaceZoneType.FREE_ZONE &&
                containsPosition(zone, az, rng, alt)) {
                return AirspaceZoneType.FREE_ZONE;
            }
        }

        return AirspaceZoneType.FREE_ZONE;
    }

    isInFreeZone(aircraft: Aircraft): boolean {
        const az = aircraft.getAzimuth();
        const rng = aircraft.getRange();
        const alt = aircraft.getAltitude();
        return this.zones.some((z) =>
            z.active && z.type === AirspaceZoneType.FREE_ZONE &&
            containsPosition(z, az, rng, alt));
    }

    isInNoFlyZone(aircraft: Aircraft): boolean {
        const az = aircraft.getAzimuth();
        const rng = aircraft.getRange();
        const alt = aircraft.getAltitude();
        return this.zones.some((z) =>
            z.active && z.type === AirspaceZoneType.NO_FLY_ZONE &&
            containsPosition(z, az, rng, alt));
    }

    matchesReturnCorridor(aircraft: Aircraft): boolean {
        const az = aircraft.getAzimuth();
        const rng = aircraft.getRange();
        const alt = aircraft.getAltitude();
        const hdg = aircraft.getHeading();
        return this.zones.some((z) =>
            z.active && z.type === AirspaceZoneType.RETURN_CORRIDOR &&
            matchesCorridor(z, az, rng, alt, hdg));
    }

    getZones(): AirspaceZone[] { return this.zones; }

    getActiveZones(): AirspaceZone[] {
        return this.zones.filter((z) => z.active);
    }

    getZone(designation: string): AirspaceZone | null {
        return this.zones.find((z) => z.designation === designation) || null;
    }

    initDefaultAirspace(): void {
        this.clearAllZones();

        this.addZone({
            designation: 'RTB-ALPHA',
            type: AirspaceZoneType.RETURN_CORRIDOR,
            startAzimuth: 210.0, endAzimuth: 240.0,
            minRange: 30.0, maxRange: 463.0,
            minAltitude: 20000.0, maxAltitude: 40000.0,
            allowedHeadingMin: 25.0, allowedHeadingMax: 65.0,
            hasHeadingConstraint: true, active: true,
        });

        this.addZone({
            designation: 'RTB-BRAVO',
            type: AirspaceZoneType.RETURN_CORRIDOR,
            startAzimuth: 300.0, endAzimuth: 330.0,
            minRange: 30.0, maxRange: 463.0,
            minAltitude: 20000.0, maxAltitude: 40000.0,
            allowedHeadingMin: 115.0, allowedHeadingMax: 155.0,
            hasHeadingConstraint: true, active: true,
        });

        this.addZone({
            designation: 'NFZ-EAST',
            type: AirspaceZoneType.NO_FLY_ZONE,
            startAzimuth: 80.0, endAzimuth: 100.0,
            minRange: 0.0, maxRange: 463.0,
            minAltitude: 0.0, maxAltitude: 10000.0,
            allowedHeadingMin: 0.0, allowedHeadingMax: 0.0,
            hasHeadingConstraint: false, active: true,
        });
    }

    static azimuthInSector(azimuth: number, start: number, end: number): boolean {
        const normalize = (a: number): number => {
            while (a < 0) a += 360;
            while (a >= 360) a -= 360;
            return a;
        };

        azimuth = normalize(azimuth);
        start = normalize(start);
        end = normalize(end);

        if (start <= end) {
            return azimuth >= start && azimuth <= end;
        }
        return azimuth >= start || azimuth <= end;
    }
}
