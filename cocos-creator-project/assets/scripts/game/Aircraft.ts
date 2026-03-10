import {
    AircraftType, IFFStatus, TrackClassification, TrackData,
    GameConstants,
} from './GameTypes';

// ============================================================================
// Aircraft entity - represents a single aircraft on the radar
// ============================================================================

export class Aircraft {
    private type: AircraftType;
    private range: number;
    private azimuth: number;
    private altitude: number;
    private speed: number;
    private heading: number;
    private friendly: boolean;
    private alive: boolean = true;
    private _reachedTerritory: boolean = false;
    private _trackId: number = -1;
    private _iffStatus: IFFStatus = IFFStatus.PENDING;
    private _timeSinceLastSweep: number = 0;

    constructor(
        type: AircraftType,
        startRange: number,
        startAzimuth: number,
        altitude: number,
        speed: number,
        heading: number,
        isFriendly: boolean,
    ) {
        this.type = type;
        this.range = startRange;
        this.azimuth = startAzimuth;
        this.altitude = altitude;
        this.speed = speed;
        this.heading = heading;
        this.friendly = isFriendly;
    }

    update(dt: number): void {
        if (!this.alive) return;
        this.updatePosition(dt);
        this.checkTerritoryPenetration();
        this._timeSinceLastSweep += dt;
    }

    private updatePosition(dt: number): void {
        const speedKmPerSec = this.speed * 0.000514444;
        const distanceKm = speedKmPerSec * dt;

        const headingRad = (this.heading * Math.PI) / 180.0;
        const azimuthRad = (this.azimuth * Math.PI) / 180.0;

        const currentX = this.range * Math.sin(azimuthRad);
        const currentY = this.range * Math.cos(azimuthRad);

        const dx = distanceKm * Math.sin(headingRad);
        const dy = distanceKm * Math.cos(headingRad);

        const newX = currentX + dx;
        const newY = currentY + dy;

        this.range = Math.sqrt(newX * newX + newY * newY);
        this.azimuth = (Math.atan2(newX, newY) * 180.0) / Math.PI;
        if (this.azimuth < 0) this.azimuth += 360.0;
    }

    private checkTerritoryPenetration(): void {
        if (this.range <= GameConstants.TERRITORY_RADIUS_KM && !this.friendly) {
            this._reachedTerritory = true;
        }
    }

    // Getters
    getType(): AircraftType { return this.type; }
    getRange(): number { return this.range; }
    getAzimuth(): number { return this.azimuth; }
    getAltitude(): number { return this.altitude; }
    getSpeed(): number { return this.speed; }
    getHeading(): number { return this.heading; }
    isFriendly(): boolean { return this.friendly; }
    isAlive(): boolean { return this.alive; }
    hasReachedTerritory(): boolean { return this._reachedTerritory; }

    getTrackId(): number { return this._trackId; }
    setTrackId(id: number): void { this._trackId = id; }

    getIFFStatus(): IFFStatus { return this._iffStatus; }
    setIFFStatus(status: IFFStatus): void { this._iffStatus = status; }

    getTimeSinceLastSweep(): number { return this._timeSinceLastSweep; }
    resetSweepTimer(): void { this._timeSinceLastSweep = 0; }
    updateSweepTimer(dt: number): void { this._timeSinceLastSweep += dt; }

    destroy(): void { this.alive = false; }
    isInRange(maxRange: number): boolean { return this.range <= maxRange; }

    getRadarCrossSection(): number {
        switch (this.type) {
            case AircraftType.STRATEGIC_BOMBER:  return 100.0;
            case AircraftType.TACTICAL_BOMBER:   return 50.0;
            case AircraftType.FIGHTER_ATTACK:    return 10.0;
            case AircraftType.RECON_DRONE:       return 1.0;
            case AircraftType.ATTACK_DRONE:      return 2.0;
            case AircraftType.STEALTH_FIGHTER:   return 0.1;
            case AircraftType.CIVILIAN_AIRLINER: return 80.0;
            case AircraftType.FRIENDLY_MILITARY: return 15.0;
        }
        return 10.0;
    }

    getTrackData(): TrackData {
        let classification: TrackClassification;
        switch (this._iffStatus) {
            case IFFStatus.FRIENDLY: classification = TrackClassification.FRIENDLY; break;
            case IFFStatus.HOSTILE:  classification = TrackClassification.HOSTILE; break;
            case IFFStatus.UNKNOWN:  classification = TrackClassification.UNKNOWN; break;
            case IFFStatus.PENDING:  classification = TrackClassification.PENDING; break;
        }

        return {
            trackId: this._trackId,
            altitude: this.altitude,
            azimuth: this.azimuth,
            range: this.range,
            speed: this.speed,
            heading: this.heading,
            iffStatus: this._iffStatus,
            classification,
            aircraftType: this.type,
            isAlive: this.alive,
            timeSinceLastSweep: this._timeSinceLastSweep,
        };
    }

    getThreatScore(): number {
        if (this.friendly) return 0;

        let score = 0;
        switch (this.type) {
            case AircraftType.STRATEGIC_BOMBER: score = 50; break;
            case AircraftType.FIGHTER_ATTACK:   score = 40; break;
            case AircraftType.STEALTH_FIGHTER:  score = 60; break;
            case AircraftType.TACTICAL_BOMBER:  score = 35; break;
            case AircraftType.ATTACK_DRONE:     score = 25; break;
            case AircraftType.RECON_DRONE:      score = 10; break;
            default: score = 0; break;
        }

        if (this.range < 30.0) score += 30;
        else if (this.range < 50.0) score += 15;

        if (this.speed > 600.0) score += 15;

        return score;
    }

    getTypeName(): string {
        switch (this.type) {
            case AircraftType.STRATEGIC_BOMBER:  return 'STRAT BOMBER';
            case AircraftType.FIGHTER_ATTACK:    return 'FIGHTER/ATK';
            case AircraftType.TACTICAL_BOMBER:   return 'TAC BOMBER';
            case AircraftType.RECON_DRONE:       return 'RECON UAV';
            case AircraftType.ATTACK_DRONE:      return 'ATTACK UAV';
            case AircraftType.STEALTH_FIGHTER:   return 'STEALTH FTR';
            case AircraftType.CIVILIAN_AIRLINER: return 'CIVILIAN';
            case AircraftType.FRIENDLY_MILITARY: return 'FRIENDLY MIL';
        }
        return 'UNKNOWN';
    }
}
