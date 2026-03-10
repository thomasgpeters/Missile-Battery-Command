import { AircraftType, GameConstants } from './GameTypes';
import { Aircraft } from './Aircraft';
import { GameConfig } from './GameConfig';

// ============================================================================
// Random aircraft generator - spawns aircraft based on game level
// ============================================================================

export class AircraftGenerator {
    private level: number = 1;
    private spawnTimer: number = 0;
    private nextSpawnTime: number = 5.0;

    init(level: number): void {
        this.level = level;
        this.spawnTimer = 0;
        this.scheduleNextSpawn();
    }

    reset(): void {
        this.spawnTimer = 0;
        this.scheduleNextSpawn();
    }

    setLevel(level: number): void {
        this.level = level;
    }

    getLevel(): number { return this.level; }

    trySpawn(dt: number, currentCount: number): Aircraft | null {
        const config = GameConfig.getInstance();
        config.setLevel(this.level);

        if (currentCount >= config.getMaxConcurrentAircraft()) {
            return null;
        }

        this.spawnTimer += dt;
        if (this.spawnTimer < this.nextSpawnTime) {
            return null;
        }

        this.spawnTimer = 0;
        this.scheduleNextSpawn();

        let type = this.selectAircraftType();
        let friendly = this.decideFriendly();

        if (friendly) {
            type = Math.random() < 0.5
                ? AircraftType.CIVILIAN_AIRLINER
                : AircraftType.FRIENDLY_MILITARY;
        }

        const startAzimuth = this.randomRange(0, 360);
        const startRange = this.generateStartRange();
        const altitude = this.generateAltitude(type);
        const speed = this.generateSpeed(type) * config.getSpeedMultiplier();
        const heading = this.generateHeading(startAzimuth);

        return new Aircraft(type, startRange, startAzimuth, altitude, speed, heading, friendly);
    }

    private selectAircraftType(): AircraftType {
        const config = GameConfig.getInstance();

        interface TypeWeight { type: AircraftType; weight: number; }
        const weights: TypeWeight[] = [
            { type: AircraftType.RECON_DRONE,      weight: 30 },
            { type: AircraftType.ATTACK_DRONE,     weight: 25 },
            { type: AircraftType.FIGHTER_ATTACK,   weight: 20 },
            { type: AircraftType.TACTICAL_BOMBER,  weight: 15 },
            { type: AircraftType.STRATEGIC_BOMBER, weight: 10 },
        ];

        if (config.isStealthEnabled()) {
            weights.push({ type: AircraftType.STEALTH_FIGHTER, weight: 8 });
        }

        let totalWeight = 0;
        for (const w of weights) totalWeight += w.weight;

        let roll = Math.floor(Math.random() * totalWeight);
        let cumulative = 0;
        for (const w of weights) {
            cumulative += w.weight;
            if (roll < cumulative) return w.type;
        }

        return AircraftType.FIGHTER_ATTACK;
    }

    private decideFriendly(): boolean {
        return Math.random() < GameConfig.getInstance().getFriendlyRatio();
    }

    private randomRange(min: number, max: number): number {
        return min + Math.random() * (max - min);
    }

    private generateStartRange(): number {
        const maxRange = GameConstants.RADAR_MAX_RANGE_KM;
        return this.randomRange(maxRange * 0.85, maxRange);
    }

    private generateAltitude(type: AircraftType): number {
        switch (type) {
            case AircraftType.STRATEGIC_BOMBER:  return this.randomRange(25000, 45000);
            case AircraftType.FIGHTER_ATTACK:    return this.randomRange(5000, 35000);
            case AircraftType.TACTICAL_BOMBER:   return this.randomRange(15000, 30000);
            case AircraftType.RECON_DRONE:       return this.randomRange(10000, 50000);
            case AircraftType.ATTACK_DRONE:      return this.randomRange(500, 15000);
            case AircraftType.STEALTH_FIGHTER:   return this.randomRange(15000, 40000);
            case AircraftType.CIVILIAN_AIRLINER: return this.randomRange(28000, 42000);
            case AircraftType.FRIENDLY_MILITARY: return this.randomRange(10000, 35000);
        }
        return 20000;
    }

    private generateSpeed(type: AircraftType): number {
        switch (type) {
            case AircraftType.STRATEGIC_BOMBER:  return this.randomRange(400, 600);
            case AircraftType.FIGHTER_ATTACK:    return this.randomRange(500, 900);
            case AircraftType.TACTICAL_BOMBER:   return this.randomRange(350, 500);
            case AircraftType.RECON_DRONE:       return this.randomRange(100, 200);
            case AircraftType.ATTACK_DRONE:      return this.randomRange(150, 300);
            case AircraftType.STEALTH_FIGHTER:   return this.randomRange(500, 800);
            case AircraftType.CIVILIAN_AIRLINER: return this.randomRange(400, 500);
            case AircraftType.FRIENDLY_MILITARY: return this.randomRange(350, 600);
        }
        return 400;
    }

    private generateHeading(startAzimuth: number): number {
        let towardCenter = startAzimuth + 180.0;
        if (towardCenter >= 360.0) towardCenter -= 360.0;

        const deviation = this.randomRange(-45.0, 45.0);
        let heading = towardCenter + deviation;
        if (heading < 0) heading += 360.0;
        if (heading >= 360.0) heading -= 360.0;

        return heading;
    }

    private scheduleNextSpawn(): void {
        this.nextSpawnTime = GameConfig.getInstance().getSpawnInterval();
    }
}
