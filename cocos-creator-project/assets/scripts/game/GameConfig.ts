import { AircraftType, GameConstants } from './GameTypes';

// ============================================================================
// Game difficulty level configuration
// ============================================================================

export interface LevelConfig {
    level: number;
    maxConcurrentAircraft: number;
    spawnIntervalMin: number;
    spawnIntervalMax: number;
    stealthEnabled: boolean;
    iffErrorRate: number;
    speedMultiplier: number;
    friendlyRatio: number;
    hostileScoreBonus: number;
}

const LEVEL_CONFIGS: LevelConfig[] = [
    { level: 1, maxConcurrentAircraft: 3,  spawnIntervalMin: 8.0, spawnIntervalMax: 15.0, stealthEnabled: false, iffErrorRate: 0.00, speedMultiplier: 0.7,  friendlyRatio: 0.30, hostileScoreBonus: 0 },
    { level: 2, maxConcurrentAircraft: 5,  spawnIntervalMin: 6.0, spawnIntervalMax: 12.0, stealthEnabled: false, iffErrorRate: 0.00, speedMultiplier: 0.85, friendlyRatio: 0.25, hostileScoreBonus: 25 },
    { level: 3, maxConcurrentAircraft: 8,  spawnIntervalMin: 4.0, spawnIntervalMax: 9.0,  stealthEnabled: false, iffErrorRate: 0.05, speedMultiplier: 1.0,  friendlyRatio: 0.20, hostileScoreBonus: 50 },
    { level: 4, maxConcurrentAircraft: 10, spawnIntervalMin: 3.0, spawnIntervalMax: 7.0,  stealthEnabled: true,  iffErrorRate: 0.10, speedMultiplier: 1.15, friendlyRatio: 0.15, hostileScoreBonus: 75 },
    { level: 5, maxConcurrentAircraft: 15, spawnIntervalMin: 2.0, spawnIntervalMax: 5.0,  stealthEnabled: true,  iffErrorRate: 0.20, speedMultiplier: 1.3,  friendlyRatio: 0.10, hostileScoreBonus: 150 },
];

const MAX_LEVEL = 5;

export class GameConfig {
    private static instance: GameConfig | null = null;
    private currentLevel: number = 1;

    static getInstance(): GameConfig {
        if (!GameConfig.instance) {
            GameConfig.instance = new GameConfig();
        }
        return GameConfig.instance;
    }

    setLevel(level: number): void {
        this.currentLevel = Math.max(1, Math.min(level, MAX_LEVEL));
    }

    getLevel(): number { return this.currentLevel; }

    getLevelConfig(): LevelConfig {
        return LEVEL_CONFIGS[this.currentLevel - 1];
    }

    getMaxConcurrentAircraft(): number { return this.getLevelConfig().maxConcurrentAircraft; }

    getSpawnInterval(): number {
        const cfg = this.getLevelConfig();
        return cfg.spawnIntervalMin + Math.random() * (cfg.spawnIntervalMax - cfg.spawnIntervalMin);
    }

    isStealthEnabled(): boolean { return this.getLevelConfig().stealthEnabled; }
    getIFFErrorRate(): number { return this.getLevelConfig().iffErrorRate; }
    getSpeedMultiplier(): number { return this.getLevelConfig().speedMultiplier; }
    getFriendlyRatio(): number { return this.getLevelConfig().friendlyRatio; }

    getHostileDestroyedScore(type: AircraftType): number {
        const cfg = this.getLevelConfig();
        let baseScore: number;

        switch (type) {
            case AircraftType.STRATEGIC_BOMBER: baseScore = 500; break;
            case AircraftType.FIGHTER_ATTACK:   baseScore = 300; break;
            case AircraftType.TACTICAL_BOMBER:  baseScore = 250; break;
            case AircraftType.ATTACK_DRONE:     baseScore = 200; break;
            case AircraftType.RECON_DRONE:      baseScore = 100; break;
            case AircraftType.STEALTH_FIGHTER:  baseScore = 500; break;
            default: baseScore = 100; break;
        }

        return baseScore + cfg.hostileScoreBonus;
    }
}
