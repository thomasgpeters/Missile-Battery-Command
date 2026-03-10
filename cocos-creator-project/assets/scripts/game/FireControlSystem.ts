import { BatteryType, BatteryData, EngagementRecord, EngagementResult } from './GameTypes';
import { MissileBattery } from './MissileBattery';
import { TrackManager } from './TrackManager';

// ============================================================================
// Fire Control System - manages all batteries and engagement authorization
// ============================================================================

export class FireControlSystem {
    private batteries: MissileBattery[] = [];
    private recentResults: EngagementRecord[] = [];

    init(): void {
        this.batteries = [];

        // 3 Patriot batteries in triangle formation (~15 km from center)
        this.batteries.push(new MissileBattery('PATRIOT-1', BatteryType.PATRIOT, 15.0, 0.0));
        this.batteries.push(new MissileBattery('PATRIOT-2', BatteryType.PATRIOT, 15.0, 120.0));
        this.batteries.push(new MissileBattery('PATRIOT-3', BatteryType.PATRIOT, 15.0, 240.0));

        // 3 Hawk batteries between Patriots (~8 km)
        this.batteries.push(new MissileBattery('HAWK-1', BatteryType.HAWK, 8.0, 60.0));
        this.batteries.push(new MissileBattery('HAWK-2', BatteryType.HAWK, 8.0, 180.0));
        this.batteries.push(new MissileBattery('HAWK-3', BatteryType.HAWK, 8.0, 300.0));

        // 3 Javelin MANPADS platoons (~3 km, innermost defense)
        this.batteries.push(new MissileBattery('JAVELIN-1', BatteryType.JAVELIN, 3.0, 30.0));
        this.batteries.push(new MissileBattery('JAVELIN-2', BatteryType.JAVELIN, 3.0, 150.0));
        this.batteries.push(new MissileBattery('JAVELIN-3', BatteryType.JAVELIN, 3.0, 270.0));
    }

    update(dt: number): void {
        for (const battery of this.batteries) {
            battery.update(dt);
        }
        this.checkEngagementResults();
    }

    assignTarget(batteryDesignation: string, trackId: number, trackMgr: TrackManager): boolean {
        const battery = this.getBattery(batteryDesignation);
        if (!battery) return false;

        const aircraft = trackMgr.getAircraftByTrackId(trackId);
        if (!aircraft) return false;

        return battery.canEngage(aircraft);
    }

    authorizeEngagement(batteryDesignation: string): boolean {
        const battery = this.getBattery(batteryDesignation);
        return battery !== null && battery.isReady();
    }

    abortEngagement(batteryDesignation: string): void {
        const battery = this.getBattery(batteryDesignation);
        if (battery) battery.abortEngagement();
    }

    getBattery(designation: string): MissileBattery | null {
        return this.batteries.find((b) => b.getDesignation() === designation) || null;
    }

    getAllBatteryData(): BatteryData[] {
        return this.batteries.map((b) => b.getData());
    }

    getAvailableBatteries(trackId: number, trackMgr: TrackManager): string[] {
        const aircraft = trackMgr.getAircraftByTrackId(trackId);
        if (!aircraft) return [];

        return this.batteries
            .filter((b) => b.canEngage(aircraft))
            .map((b) => b.getDesignation());
    }

    getRecentResults(): EngagementRecord[] {
        const results = [...this.recentResults];
        this.recentResults = [];
        return results;
    }

    getBatteries(): MissileBattery[] {
        return this.batteries;
    }

    reset(): void {
        this.batteries = [];
        this.recentResults = [];
        this.init();
    }

    private checkEngagementResults(): void {
        for (const battery of this.batteries) {
            if (battery.hasEngagementResult()) {
                this.recentResults.push({
                    trackId: battery.getAssignedTrackId(),
                    batteryDesignation: battery.getDesignation(),
                    result: battery.getLastResult(),
                    missileFlightTime: 0,
                    elapsedTime: 0,
                });
                battery.clearEngagementResult();
            }
        }
    }
}
