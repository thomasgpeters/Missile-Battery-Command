import {
    AircraftType, BatteryType, BatteryStatus, EngagementResult,
    BatteryData, PolarCoord, GameConstants,
} from './GameTypes';
import { Aircraft } from './Aircraft';

// ============================================================================
// Missile Battery - Patriot (MPMB), Hawk (HSAMB), or Javelin (MANPADS)
// ============================================================================

export class MissileBattery {
    private designation: string;
    private type: BatteryType;
    private status: BatteryStatus = BatteryStatus.READY;
    private missilesRemaining: number;
    private maxMissiles: number;
    private totalMissileStock: number;
    private loaderCount: number;
    private reloadTimeRemaining: number = 0;
    private reloadTime: number;
    private maxRange: number;
    private minRange: number;
    private maxAltitude: number;
    private minAltitude: number;
    private missileSpeed: number;
    private baseKillProbability: number;
    private position: PolarCoord;

    private trackingRadarType: string;
    private _hasMissileTracking: boolean;
    private maxSimultaneousEngagements: number;

    private isRelocating_: boolean = false;
    private relocateTimeRemaining_: number = 0;
    private relocateDestination: PolarCoord = { range: 0, azimuth: 0 };

    private engagementCount_: number = 0;
    private hitCount_: number = 0;
    private missCount_: number = 0;

    private assignedTrackId: number = -1;
    private currentTarget: Aircraft | null = null;
    private missileFlightTime: number = 0;
    private missileFlightElapsed: number = 0;
    private lastResult: EngagementResult = EngagementResult.ABORTED;
    private hasResult_: boolean = false;

    constructor(designation: string, type: BatteryType, posRange: number, posAzimuth: number) {
        this.designation = designation;
        this.type = type;
        this.position = { range: posRange, azimuth: posAzimuth };

        if (type === BatteryType.PATRIOT) {
            this.maxMissiles = GameConstants.PATRIOT_MAX_MISSILES;
            this.missilesRemaining = this.maxMissiles;
            this.totalMissileStock = 0;
            this.loaderCount = 1;
            this.reloadTime = GameConstants.PATRIOT_RELOAD_TIME;
            this.maxRange = GameConstants.PATRIOT_MAX_RANGE;
            this.minRange = GameConstants.PATRIOT_MIN_RANGE;
            this.maxAltitude = GameConstants.PATRIOT_MAX_ALT;
            this.minAltitude = GameConstants.PATRIOT_MIN_ALT;
            this.missileSpeed = GameConstants.PATRIOT_MISSILE_SPEED;
            this.baseKillProbability = GameConstants.PATRIOT_KILL_PROB;
            this.trackingRadarType = 'AN/MPQ-53';
            this._hasMissileTracking = true;
            this.maxSimultaneousEngagements = 1;
        } else if (type === BatteryType.HAWK) {
            this.maxMissiles = GameConstants.HAWK_MAX_MISSILES;
            this.missilesRemaining = this.maxMissiles;
            this.totalMissileStock = GameConstants.HAWK_TOTAL_STOCK - this.maxMissiles;
            this.loaderCount = GameConstants.HAWK_LOADERS;
            this.reloadTime = GameConstants.HAWK_RELOAD_TIME;
            this.maxRange = GameConstants.HAWK_MAX_RANGE;
            this.minRange = GameConstants.HAWK_MIN_RANGE;
            this.maxAltitude = GameConstants.HAWK_MAX_ALT;
            this.minAltitude = GameConstants.HAWK_MIN_ALT;
            this.missileSpeed = GameConstants.HAWK_MISSILE_SPEED;
            this.baseKillProbability = GameConstants.HAWK_KILL_PROB;
            this.trackingRadarType = 'AN/MPQ-46 HPI';
            this._hasMissileTracking = true;
            this.maxSimultaneousEngagements = 1;
        } else {
            this.maxMissiles = GameConstants.JAVELIN_MAX_MISSILES;
            this.missilesRemaining = this.maxMissiles;
            this.totalMissileStock = 0;
            this.loaderCount = 1;
            this.reloadTime = GameConstants.JAVELIN_RELOAD_TIME;
            this.maxRange = GameConstants.JAVELIN_MAX_RANGE;
            this.minRange = GameConstants.JAVELIN_MIN_RANGE;
            this.maxAltitude = GameConstants.JAVELIN_MAX_ALT;
            this.minAltitude = GameConstants.JAVELIN_MIN_ALT;
            this.missileSpeed = GameConstants.JAVELIN_MISSILE_SPEED;
            this.baseKillProbability = GameConstants.JAVELIN_KILL_PROB;
            this.trackingRadarType = 'CLU IR/FLIR';
            this._hasMissileTracking = false;
            this.maxSimultaneousEngagements = 1;
        }
    }

    update(dt: number): void {
        switch (this.status) {
            case BatteryStatus.ENGAGED: {
                this.missileFlightElapsed += dt;
                if (this.missileFlightElapsed >= this.missileFlightTime) {
                    const killProb = this.calculateKillProbability(this.currentTarget);
                    this.engagementCount_++;

                    if (this.currentTarget && this.currentTarget.isAlive() &&
                        Math.random() < killProb) {
                        this.lastResult = EngagementResult.HIT;
                        this.currentTarget.destroy();
                        this.hitCount_++;
                    } else {
                        this.lastResult = EngagementResult.MISS;
                        this.missCount_++;
                    }

                    this.hasResult_ = true;
                    this.currentTarget = null;
                    this.assignedTrackId = -1;

                    if (this.missilesRemaining <= 0) {
                        const toReload = Math.min(this.maxMissiles, this.totalMissileStock);
                        if (toReload > 0) {
                            this.status = BatteryStatus.RELOADING;
                            this.reloadTimeRemaining = this.reloadTime;
                            this.missilesRemaining = toReload;
                            this.totalMissileStock -= toReload;
                        } else {
                            this.status = BatteryStatus.OFFLINE;
                        }
                    } else {
                        this.status = BatteryStatus.READY;
                    }
                }
                break;
            }

            case BatteryStatus.RELOADING: {
                this.reloadTimeRemaining -= dt;
                if (this.reloadTimeRemaining <= 0) {
                    this.reloadTimeRemaining = 0;
                    this.status = BatteryStatus.READY;
                }
                break;
            }

            case BatteryStatus.OFFLINE: {
                if (this.isRelocating_) {
                    this.relocateTimeRemaining_ -= dt;
                    if (this.relocateTimeRemaining_ <= 0) {
                        this.position = { ...this.relocateDestination };
                        this.isRelocating_ = false;
                        this.relocateTimeRemaining_ = 0;
                        this.status = BatteryStatus.READY;
                    }
                }
                break;
            }
        }
    }

    engage(target: Aircraft): boolean {
        if (!this.canEngage(target)) return false;
        if (this.status !== BatteryStatus.READY && this.status !== BatteryStatus.TRACKING) {
            return false;
        }

        this.currentTarget = target;
        this.assignedTrackId = target.getTrackId();
        this.missilesRemaining--;
        this.missileFlightTime = this.calculateFlightTime(target);
        this.missileFlightElapsed = 0;
        this.status = BatteryStatus.ENGAGED;
        this.hasResult_ = false;
        return true;
    }

    abortEngagement(): void {
        if (this.status === BatteryStatus.TRACKING) {
            this.status = BatteryStatus.READY;
            this.currentTarget = null;
            this.assignedTrackId = -1;
        }
    }

    canEngage(target: Aircraft | null): boolean {
        if (!target || !target.isAlive()) return false;
        if (this.status !== BatteryStatus.READY && this.status !== BatteryStatus.TRACKING) {
            return false;
        }
        if (this.missilesRemaining <= 0) return false;

        const range = target.getRange();
        const alt = target.getAltitude();

        return range >= this.minRange && range <= this.maxRange &&
               alt >= this.minAltitude && alt <= this.maxAltitude;
    }

    getData(): BatteryData {
        return {
            designation: this.designation,
            type: this.type,
            status: this.status,
            missilesRemaining: this.missilesRemaining,
            maxMissiles: this.maxMissiles,
            totalMissileStock: this.totalMissileStock + this.missilesRemaining,
            loaderCount: this.loaderCount,
            reloadTimeRemaining: this.reloadTimeRemaining,
            maxRange: this.maxRange,
            minRange: this.minRange,
            maxAltitude: this.maxAltitude,
            minAltitude: this.minAltitude,
            assignedTrackId: this.assignedTrackId,
            position: { ...this.position },
            trackingRadarType: this.trackingRadarType,
            hasMissileTracking: this._hasMissileTracking,
            maxSimultaneousEngagements: this.maxSimultaneousEngagements,
        };
    }

    relocate(newRange: number, newAzimuth: number): boolean {
        if (this.status === BatteryStatus.ENGAGED) return false;
        if (this.isRelocating_) return false;
        if (this.status === BatteryStatus.DESTROYED) return false;

        this.relocateDestination = { range: newRange, azimuth: newAzimuth };
        this.isRelocating_ = true;

        switch (this.type) {
            case BatteryType.PATRIOT: this.relocateTimeRemaining_ = 60.0; break;
            case BatteryType.HAWK:    this.relocateTimeRemaining_ = 45.0; break;
            case BatteryType.JAVELIN: this.relocateTimeRemaining_ = 15.0; break;
        }

        this.status = BatteryStatus.OFFLINE;
        this.currentTarget = null;
        this.assignedTrackId = -1;
        return true;
    }

    // Getters
    getDesignation(): string { return this.designation; }
    getType(): BatteryType { return this.type; }
    getStatus(): BatteryStatus { return this.status; }
    getMissilesRemaining(): number { return this.missilesRemaining; }
    getAssignedTrackId(): number { return this.assignedTrackId; }
    isReady(): boolean { return this.status === BatteryStatus.READY; }
    getLastResult(): EngagementResult { return this.lastResult; }
    hasEngagementResult(): boolean { return this.hasResult_; }
    clearEngagementResult(): void { this.hasResult_ = false; }
    isRelocating(): boolean { return this.isRelocating_; }
    getRelocateTimeRemaining(): number { return this.relocateTimeRemaining_; }
    getPosition(): PolarCoord { return { ...this.position }; }
    getRelocateDestination(): PolarCoord { return { ...this.relocateDestination }; }
    getEngagementCount(): number { return this.engagementCount_; }
    getHitCount(): number { return this.hitCount_; }
    getMissCount(): number { return this.missCount_; }
    getTotalMissileStock(): number { return this.totalMissileStock; }
    getLoaderCount(): number { return this.loaderCount; }
    getTrackingRadarType(): string { return this.trackingRadarType; }
    hasMissileTracking(): boolean { return this._hasMissileTracking; }
    getMaxSimultaneousEngagements(): number { return this.maxSimultaneousEngagements; }

    private calculateKillProbability(target: Aircraft | null): number {
        if (!target) return 0;

        let prob = this.baseKillProbability;

        if (target.getType() === AircraftType.STEALTH_FIGHTER) {
            prob *= 0.6;
        }

        if (target.getSpeed() > 700) {
            prob *= 0.85;
        }

        if (this.type === BatteryType.PATRIOT && target.getAltitude() < 5000) {
            prob *= 0.7;
        }

        if (this.type === BatteryType.HAWK &&
            (target.getType() === AircraftType.ATTACK_DRONE ||
             target.getType() === AircraftType.RECON_DRONE)) {
            prob *= 1.1;
        }

        if (this.type === BatteryType.JAVELIN) {
            if (target.getType() === AircraftType.ATTACK_DRONE) {
                prob *= 1.2;
            }
            if (target.getSpeed() > 500) {
                prob *= 0.5;
            }
            if (target.getType() === AircraftType.STEALTH_FIGHTER) {
                prob *= 1.3;
            }
        }

        const rangeFraction = target.getRange() / this.maxRange;
        if (rangeFraction > 0.8) {
            prob *= 0.9;
        }

        return Math.min(prob, 0.95);
    }

    private calculateFlightTime(target: Aircraft): number {
        const distanceKm = target.getRange();
        const speedKmPerSec = this.missileSpeed / 1000.0;
        return distanceKm / speedKmPerSec;
    }
}
