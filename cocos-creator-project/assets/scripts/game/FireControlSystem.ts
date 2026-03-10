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

        // Realistic battalion spread: ~75 km across (batteries ~25 km apart)
        // Each battery has its own organic radar and operates autonomously
        // when HQ is offline. Positions are relative to the defended asset.

        // 3 Patriot batteries in triangle formation (~35 km from center)
        // Long-range / high-altitude umbrella — AN/MPQ-53 phased array
        // Each Patriot covers 160 km range, so at 35 km out they overlap
        // heavily and provide layered coverage over the entire zone.
        this.batteries.push(new MissileBattery('PATRIOT-1', BatteryType.PATRIOT, 35.0, 0.0));
        this.batteries.push(new MissileBattery('PATRIOT-2', BatteryType.PATRIOT, 35.0, 120.0));
        this.batteries.push(new MissileBattery('PATRIOT-3', BatteryType.PATRIOT, 35.0, 240.0));

        // 3 Hawk batteries between the Patriots (~20 km from center)
        // Medium-range / low-altitude defense — AN/MPQ-46 HPI radar
        // Covers the low-altitude corridor that Patriot can't reach,
        // and fills gaps between the Patriot positions.
        this.batteries.push(new MissileBattery('HAWK-1', BatteryType.HAWK, 20.0, 60.0));
        this.batteries.push(new MissileBattery('HAWK-2', BatteryType.HAWK, 20.0, 180.0));
        this.batteries.push(new MissileBattery('HAWK-3', BatteryType.HAWK, 20.0, 300.0));

        // 3 Javelin MANPADS platoons (~8 km from center, inner ring)
        // Last line of defense — CLU IR/FLIR seeker, no radar signature
        // Close-in protection for the asset itself.
        this.batteries.push(new MissileBattery('JAVELIN-1', BatteryType.JAVELIN, 8.0, 30.0));
        this.batteries.push(new MissileBattery('JAVELIN-2', BatteryType.JAVELIN, 8.0, 150.0));
        this.batteries.push(new MissileBattery('JAVELIN-3', BatteryType.JAVELIN, 8.0, 270.0));
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
