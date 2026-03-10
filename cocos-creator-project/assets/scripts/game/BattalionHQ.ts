import { PolarCoord } from './GameTypes';

// ============================================================================
// Battalion HQ — Mobile AN/TSQ-73 Missile Minder Battery
// ============================================================================

export enum HQStatus {
    OPERATIONAL,
    RELOCATING,
    SETTING_UP,
    DEGRADED,
}

export interface HQData {
    designation: string;
    status: HQStatus;
    position: PolarCoord;
    relocateDestination: PolarCoord;
    isRelocating: boolean;
    relocateTimeRemaining: number;
    totalRelocateTime: number;
    personnelCount: number;
    radarOnline: boolean;
    commsOnline: boolean;
    statusString: string;
}

export class BattalionHQ {
    // Real-world relocation timing (in game seconds, representing minutes)
    // Actual AN/TSQ-73 battalion displacement: ~45 min teardown, travel,
    // ~45 min setup. Total 90+ minutes plus convoy travel time.
    // Game uses 1 game-second = 1 real-minute time compression for relocation.
    static readonly TEARDOWN_TIME = 45.0;   // 45 minutes — power down, cables, antenna
    static readonly MOVE_TIME = 30.0;        // 30 minutes — convoy travel (variable)
    static readonly SETUP_TIME = 45.0;       // 45 minutes — erect antenna, cables, boot L-112s
    static readonly TOTAL_RELOCATE_TIME = 120.0; // ~2 hours total displacement
    static readonly HQ_PERSONNEL = 35;

    private designation = 'MINDER-HQ';
    private status: HQStatus = HQStatus.OPERATIONAL;
    private position: PolarCoord = { range: 0, azimuth: 0 };
    private relocateDestination: PolarCoord = { range: 0, azimuth: 0 };
    private _isRelocating = false;
    private relocateTimeRemaining = 0;
    private _radarOnline = false;
    private _commsOnline = false;

    init(posRange: number, posAzimuth: number): void {
        this.position = { range: posRange, azimuth: posAzimuth };
        this.status = HQStatus.OPERATIONAL;
        this._isRelocating = false;
        this.relocateTimeRemaining = 0;
        this._radarOnline = true;
        this._commsOnline = true;
    }

    update(dt: number): void {
        if (!this._isRelocating) return;

        this.relocateTimeRemaining -= dt;

        if (this.relocateTimeRemaining <= 0) {
            this.position = { ...this.relocateDestination };
            this._isRelocating = false;
            this.relocateTimeRemaining = 0;
            this.status = HQStatus.OPERATIONAL;
            this._radarOnline = true;
            this._commsOnline = true;
        } else if (this.relocateTimeRemaining <= BattalionHQ.SETUP_TIME) {
            this.status = HQStatus.SETTING_UP;
            this._radarOnline = this.relocateTimeRemaining <= BattalionHQ.SETUP_TIME * 0.5;
            this._commsOnline = false;
        } else {
            this.status = HQStatus.RELOCATING;
            this._radarOnline = false;
            this._commsOnline = false;
        }
    }

    relocate(newRange: number, newAzimuth: number): boolean {
        if (this._isRelocating) return false;
        if (this.status === HQStatus.SETTING_UP) return false;

        this.relocateDestination = { range: newRange, azimuth: newAzimuth };
        this._isRelocating = true;
        this.relocateTimeRemaining = BattalionHQ.TOTAL_RELOCATE_TIME;
        this.status = HQStatus.RELOCATING;
        this._radarOnline = false;
        this._commsOnline = false;
        return true;
    }

    cancelRelocation(): void {
        if (!this._isRelocating) return;

        this._isRelocating = false;
        this.relocateTimeRemaining = 0;
        this.status = HQStatus.SETTING_UP;
        this.relocateTimeRemaining = BattalionHQ.SETUP_TIME;
        this._isRelocating = true;
        this.relocateDestination = { ...this.position };
    }

    // Getters
    getStatus(): HQStatus { return this.status; }
    isOperational(): boolean { return this.status === HQStatus.OPERATIONAL; }
    isRelocating(): boolean { return this._isRelocating; }
    isRadarOnline(): boolean { return this._radarOnline; }
    isCommsOnline(): boolean { return this._commsOnline; }
    getPosition(): PolarCoord { return { ...this.position }; }
    getRelocateTimeRemaining(): number { return this.relocateTimeRemaining; }

    getStatusString(): string {
        switch (this.status) {
            case HQStatus.OPERATIONAL: return 'OPERATIONAL';
            case HQStatus.RELOCATING:  return 'RELOCATING';
            case HQStatus.SETTING_UP:  return 'SETTING UP';
            case HQStatus.DEGRADED:    return 'DEGRADED';
        }
    }

    getData(): HQData {
        return {
            designation: this.designation,
            status: this.status,
            position: { ...this.position },
            relocateDestination: { ...this.relocateDestination },
            isRelocating: this._isRelocating,
            relocateTimeRemaining: this.relocateTimeRemaining,
            totalRelocateTime: BattalionHQ.TOTAL_RELOCATE_TIME,
            personnelCount: BattalionHQ.HQ_PERSONNEL,
            radarOnline: this._radarOnline,
            commsOnline: this._commsOnline,
            statusString: this.getStatusString(),
        };
    }
}
