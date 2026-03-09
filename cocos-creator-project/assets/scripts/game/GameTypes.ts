// ============================================================================
// Core Enumerations
// ============================================================================

export enum AircraftType {
    STRATEGIC_BOMBER,
    FIGHTER_ATTACK,
    TACTICAL_BOMBER,
    RECON_DRONE,
    ATTACK_DRONE,
    STEALTH_FIGHTER,
    CIVILIAN_AIRLINER,
    FRIENDLY_MILITARY,
}

export enum IFFStatus {
    PENDING,
    FRIENDLY,
    HOSTILE,
    UNKNOWN,
}

export enum TrackClassification {
    PENDING,
    FRIENDLY,
    HOSTILE,
    UNKNOWN,
}

export enum BatteryType {
    PATRIOT,
    HAWK,
    JAVELIN,
}

export enum BatteryStatus {
    READY,
    TRACKING,
    ENGAGED,
    RELOADING,
    DESTROYED,
    OFFLINE,
}

export enum EngagementResult {
    HIT,
    MISS,
    IN_FLIGHT,
    ABORTED,
}

// ============================================================================
// Data Structures
// ============================================================================

export interface PolarCoord {
    range: number;
    azimuth: number;
}

export function polarToScreenX(coord: PolarCoord, scale: number): number {
    return coord.range * scale * Math.sin((coord.azimuth * Math.PI) / 180.0);
}

export function polarToScreenY(coord: PolarCoord, scale: number): number {
    return coord.range * scale * Math.cos((coord.azimuth * Math.PI) / 180.0);
}

export interface TrackData {
    trackId: number;
    altitude: number;
    azimuth: number;
    range: number;
    speed: number;
    heading: number;
    iffStatus: IFFStatus;
    classification: TrackClassification;
    aircraftType: AircraftType;
    isAlive: boolean;
    timeSinceLastSweep: number;
}

export function getTrackIdString(trackId: number): string {
    return `TK-${String(trackId).padStart(3, '0')}`;
}

export function getClassificationString(classification: TrackClassification): string {
    switch (classification) {
        case TrackClassification.FRIENDLY: return 'FRIENDLY';
        case TrackClassification.HOSTILE:  return 'HOSTILE';
        case TrackClassification.UNKNOWN:  return 'UNKNOWN';
        case TrackClassification.PENDING:  return 'PENDING';
    }
}

export function getAltitudeString(altitude: number): string {
    if (altitude >= 1000) {
        return `FL${String(Math.floor(altitude / 100)).padStart(3, '0')}`;
    }
    return `${Math.floor(altitude)}ft`;
}

export interface BatteryData {
    designation: string;
    type: BatteryType;
    status: BatteryStatus;
    missilesRemaining: number;
    maxMissiles: number;
    totalMissileStock: number;
    loaderCount: number;
    reloadTimeRemaining: number;
    maxRange: number;
    minRange: number;
    maxAltitude: number;
    minAltitude: number;
    assignedTrackId: number;
    position: PolarCoord;
    trackingRadarType: string;
    hasMissileTracking: boolean;
    maxSimultaneousEngagements: number;
}

export interface EngagementRecord {
    trackId: number;
    batteryDesignation: string;
    result: EngagementResult;
    missileFlightTime: number;
    elapsedTime: number;
}

// ============================================================================
// Game Configuration Constants
// ============================================================================

export const GameConstants = {
    // Radar — Raytheon AN/TPS-43E long-range surveillance radar
    RADAR_MAX_RANGE_NM: 250.0,
    RADAR_MAX_RANGE_KM: 463.0,
    RADAR_SWEEP_RATE_RPM: 6.0,
    RADAR_SWEEP_RATE_DPS: 6.0 * 6.0,
    RADAR_RANGE_RINGS: 5,
    BLIP_FADE_TIME: 10.0,

    // IFF
    IFF_INTERROGATION_TIME: 2.0,

    // Unit conversions
    NM_TO_KM: 1.852,
    KM_TO_NM: 1.0 / 1.852,

    // Patriot Battery (MPMB) — MIM-104
    PATRIOT_MAX_RANGE: 160.0,
    PATRIOT_MIN_RANGE: 3.0,
    PATRIOT_MAX_ALT: 80000.0,
    PATRIOT_MIN_ALT: 1000.0,
    PATRIOT_MAX_MISSILES: 4,
    PATRIOT_RELOAD_TIME: 15.0,
    PATRIOT_MISSILE_SPEED: 1700.0,
    PATRIOT_KILL_PROB: 0.85,

    // Hawk Battery (HSAMB) — MIM-23
    HAWK_MAX_RANGE: 45.0,
    HAWK_MIN_RANGE: 1.0,
    HAWK_MAX_ALT: 45000.0,
    HAWK_MIN_ALT: 100.0,
    HAWK_MAX_MISSILES: 3,
    HAWK_TOTAL_STOCK: 33,
    HAWK_LOADERS: 3,
    HAWK_RELOAD_TIME: 10.0,
    HAWK_MISSILE_SPEED: 900.0,
    HAWK_KILL_PROB: 0.75,

    // Javelin MANPADS Platoon — FGM-148
    JAVELIN_MAX_RANGE: 55.0,
    JAVELIN_MIN_RANGE: 0.5,
    JAVELIN_MAX_ALT: 15000.0,
    JAVELIN_MIN_ALT: 0.0,
    JAVELIN_MAX_MISSILES: 2,
    JAVELIN_RELOAD_TIME: 20.0,
    JAVELIN_MISSILE_SPEED: 300.0,
    JAVELIN_KILL_PROB: 0.65,

    // Scoring
    SCORE_HOSTILE_DESTROYED_BASE: 100,
    SCORE_FRIENDLY_DESTROYED: -1000,
    SCORE_HOSTILE_PENETRATED: -200,
    SCORE_MISSILE_WASTED: -25,
    SCORE_FIRST_SHOT_MULTIPLIER: 2.0,

    // Territory — inner defense zone
    TERRITORY_RADIUS_KM: 25.0,
} as const;
