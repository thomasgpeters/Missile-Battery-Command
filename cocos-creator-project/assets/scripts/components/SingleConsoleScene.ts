import { _decorator, Component, Node, input, Input, EventKeyboard, KeyCode, EventTouch, UITransform, Vec3 } from 'cc';
import { GameConstants, EngagementResult, getTrackIdString } from '../game/GameTypes';
import { Aircraft } from '../game/Aircraft';
import { AircraftGenerator } from '../game/AircraftGenerator';
import { TrackManager } from '../game/TrackManager';
import { FireControlSystem } from '../game/FireControlSystem';
import { GameConfig } from '../game/GameConfig';
import { ThreatBoard } from '../game/ThreatBoard';
import { BattalionHQ } from '../game/BattalionHQ';
import { RadarDisplay } from './RadarDisplay';
import { ConsoleFrame } from './ConsoleFrame';
import { OverheadHUD } from './OverheadHUD';

const { ccclass, property } = _decorator;

// ============================================================================
// SingleConsoleScene — one operator station view of the AN/TSQ-73
//
// Layout (matching the real console photo):
//   - ConsoleFrame: metal housing, button panels, scope bezel, knobs
//   - RadarDisplay: PPI scope centered in the console frame
//   - OverheadHUD: red LED status displays mounted above
//
// This is the primary gameplay scene — the player sits at one console.
// ============================================================================

@ccclass('SingleConsoleScene')
export class SingleConsoleScene extends Component {
    @property(RadarDisplay)
    radarDisplay: RadarDisplay | null = null;

    @property(ConsoleFrame)
    consoleFrame: ConsoleFrame | null = null;

    @property(OverheadHUD)
    overheadHUD: OverheadHUD | null = null;

    // Core game systems
    private aircraftGenerator = new AircraftGenerator();
    private trackManager = new TrackManager();
    private fireControlSystem = new FireControlSystem();
    private threatBoard = new ThreatBoard();
    private battalionHQ = new BattalionHQ();
    private gameConfig = GameConfig.getInstance();

    // Game state
    private score: number = 0;
    private gameOver: boolean = false;
    private aircraft: Aircraft[] = [];
    private messages: string[] = [];

    start(): void {
        this.gameConfig.setLevel(1);

        this.aircraftGenerator.init(this.gameConfig.getLevel());
        this.fireControlSystem.init();
        this.battalionHQ.init(0.5, 0.0);
        this.trackManager.getIFFSystem().setErrorRate(this.gameConfig.getIFFErrorRate());

        // Wire up RadarDisplay
        if (this.radarDisplay) {
            this.radarDisplay.setTrackManager(this.trackManager);
            this.radarDisplay.setFireControlSystem(this.fireControlSystem);
        }

        // Wire up OverheadHUD
        if (this.overheadHUD) {
            this.overheadHUD.setTrackManager(this.trackManager);
            this.overheadHUD.setFireControlSystem(this.fireControlSystem);
            this.overheadHUD.setThreatBoard(this.threatBoard);
            this.overheadHUD.setBattalionHQ(this.battalionHQ);
            this.overheadHUD.setLevel(this.gameConfig.getLevel());
        }

        this.initInputHandlers();
    }

    private initInputHandlers(): void {
        if (this.radarDisplay) {
            this.radarDisplay.node.on(Node.EventType.TOUCH_END, this.onRadarTouch, this);
        }
        input.on(Input.EventType.KEY_DOWN, this.onKeyDown, this);
    }

    private onRadarTouch(event: EventTouch): void {
        if (!this.radarDisplay) return;
        const uiPos = event.getUILocation();
        const transform = this.radarDisplay.node.getComponent(UITransform);
        if (!transform) return;
        const nodePos = transform.convertToNodeSpaceAR(new Vec3(uiPos.x, uiPos.y, 0));
        const nearestTrackId = this.radarDisplay.findNearestTrack(nodePos.x, nodePos.y);
        this.onTrackSelected(nearestTrackId);
    }

    private onKeyDown(event: EventKeyboard): void {
        const batteryMap: Record<number, string> = {
            [KeyCode.DIGIT_1]: 'PATRIOT-1',
            [KeyCode.DIGIT_2]: 'PATRIOT-2',
            [KeyCode.DIGIT_3]: 'PATRIOT-3',
            [KeyCode.DIGIT_4]: 'HAWK-1',
            [KeyCode.DIGIT_5]: 'HAWK-2',
            [KeyCode.DIGIT_6]: 'HAWK-3',
            [KeyCode.DIGIT_7]: 'JAVELIN-1',
            [KeyCode.DIGIT_8]: 'JAVELIN-2',
            [KeyCode.DIGIT_9]: 'JAVELIN-3',
        };

        if (batteryMap[event.keyCode]) {
            this.onBatteryAssign(batteryMap[event.keyCode]);
        } else if (event.keyCode === KeyCode.KEY_F) {
            this.onFireAuthorized();
        } else if (event.keyCode === KeyCode.KEY_A) {
            this.onAbortEngagement();
        }
    }

    update(dt: number): void {
        if (this.gameOver) return;

        this.spawnAircraft(dt);
        this.updateAircraft(dt);
        this.trackManager.update(dt);
        this.fireControlSystem.update(dt);
        this.threatBoard.update(this.trackManager);
        this.battalionHQ.update(dt);
        this.checkEngagements();
        this.cleanupDestroyedAircraft();
        this.checkGameOver();

        // Update overhead HUD score
        if (this.overheadHUD) {
            this.overheadHUD.setScore(this.score);
        }
    }

    private spawnAircraft(dt: number): void {
        const currentCount = this.aircraft.filter((ac) => ac.isAlive()).length;
        const newAircraft = this.aircraftGenerator.trySpawn(dt, currentCount);

        if (newAircraft) {
            this.trackManager.addTrack(newAircraft);
            this.aircraft.push(newAircraft);
        }
    }

    private updateAircraft(dt: number): void {
        for (const ac of this.aircraft) {
            if (ac.isAlive()) {
                ac.update(dt);
                if (ac.hasReachedTerritory() && !ac.isFriendly()) {
                    this.onHostilePenetrated(ac);
                    ac.destroy();
                }
            }
        }
    }

    private checkEngagements(): void {
        const results = this.fireControlSystem.getRecentResults();
        for (const result of results) {
            const aircraft = this.trackManager.getAircraftByTrackId(result.trackId);
            if (result.result === EngagementResult.HIT && aircraft) {
                if (aircraft.isFriendly()) {
                    this.onFriendlyDestroyed(aircraft);
                } else {
                    this.onHostileDestroyed(aircraft);
                }
            } else if (result.result === EngagementResult.MISS) {
                this.score += GameConstants.SCORE_MISSILE_WASTED;
            }
        }
    }

    private cleanupDestroyedAircraft(): void {
        this.aircraft = this.aircraft.filter((ac) =>
            ac.isAlive() && ac.getRange() <= GameConstants.RADAR_MAX_RANGE_KM * 1.1);
    }

    private checkGameOver(): void {
        if (this.score < -2000) {
            this.gameOver = true;
        }
    }

    private onTrackSelected(trackId: number): void {
        if (this.radarDisplay) this.radarDisplay.setSelectedTrack(trackId);
    }

    private onBatteryAssign(batteryDesignation: string): void {
        if (!this.radarDisplay || this.radarDisplay.getSelectedTrack() < 0) return;
        const battery = this.fireControlSystem.getBattery(batteryDesignation);
        const aircraft = this.trackManager.getAircraftByTrackId(
            this.radarDisplay.getSelectedTrack());
        if (battery && aircraft && battery.canEngage(aircraft)) {
            battery.engage(aircraft);
        }
    }

    private onFireAuthorized(): void {}
    private onAbortEngagement(): void {}

    private onHostileDestroyed(aircraft: Aircraft): void {
        this.score += this.gameConfig.getHostileDestroyedScore(aircraft.getType());
    }

    private onFriendlyDestroyed(_aircraft: Aircraft): void {
        this.score += GameConstants.SCORE_FRIENDLY_DESTROYED;
    }

    private onHostilePenetrated(_aircraft: Aircraft): void {
        this.score += GameConstants.SCORE_HOSTILE_PENETRATED;
    }

    onDestroy(): void {
        input.off(Input.EventType.KEY_DOWN, this.onKeyDown, this);
    }
}
