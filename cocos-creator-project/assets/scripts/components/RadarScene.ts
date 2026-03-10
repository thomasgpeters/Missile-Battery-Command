import { _decorator, Component, Node, EventTouch, EventKeyboard, KeyCode, input, Input } from 'cc';
import { GameConstants, EngagementResult, getTrackIdString } from '../game/GameTypes';
import { Aircraft } from '../game/Aircraft';
import { AircraftGenerator } from '../game/AircraftGenerator';
import { TrackManager } from '../game/TrackManager';
import { FireControlSystem } from '../game/FireControlSystem';
import { GameConfig } from '../game/GameConfig';
import { ThreatBoard } from '../game/ThreatBoard';
import { BattalionHQ } from '../game/BattalionHQ';
import { RadarDisplay } from './RadarDisplay';
import { GameHUD } from './GameHUD';

const { ccclass, property } = _decorator;

// ============================================================================
// RadarScene - Main game scene controller (AN/TSQ-73 console)
// ============================================================================

@ccclass('RadarScene')
export class RadarScene extends Component {
    @property(RadarDisplay)
    radarDisplay: RadarDisplay | null = null;

    @property(GameHUD)
    gameHUD: GameHUD | null = null;

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

    start(): void {
        // Initialize game config
        this.gameConfig.setLevel(1);

        // Initialize subsystems
        this.aircraftGenerator.init(this.gameConfig.getLevel());
        this.fireControlSystem.init();
        this.battalionHQ.init(0.5, 0.0);
        this.trackManager.getIFFSystem().setErrorRate(this.gameConfig.getIFFErrorRate());

        // Connect data sources to display components
        if (this.radarDisplay) {
            this.radarDisplay.setTrackManager(this.trackManager);
            this.radarDisplay.setFireControlSystem(this.fireControlSystem);
        }

        if (this.gameHUD) {
            this.gameHUD.setTrackManager(this.trackManager);
            this.gameHUD.setFireControlSystem(this.fireControlSystem);
            this.gameHUD.setThreatBoard(this.threatBoard);
            this.gameHUD.setBattalionHQ(this.battalionHQ);
            this.gameHUD.setLevel(this.gameConfig.getLevel());
        }

        // Input handlers
        this.initInputHandlers();
    }

    private initInputHandlers(): void {
        // Touch/click on radar to select tracks
        if (this.radarDisplay) {
            this.radarDisplay.node.on(Node.EventType.TOUCH_END, this.onRadarTouch, this);
        }

        // Keyboard for battery assignment and fire control
        input.on(Input.EventType.KEY_DOWN, this.onKeyDown, this);
    }

    private onRadarTouch(event: EventTouch): void {
        if (!this.radarDisplay) return;

        const uiPos = event.getUILocation();
        const nodePos = this.radarDisplay.node.getComponent('cc.UITransform')!
            .convertToNodeSpaceAR(new (require('cc').Vec3)(uiPos.x, uiPos.y, 0));

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
    }

    private spawnAircraft(dt: number): void {
        const currentCount = this.aircraft.filter((ac) => ac.isAlive()).length;
        const newAircraft = this.aircraftGenerator.trySpawn(dt, currentCount);

        if (newAircraft) {
            this.trackManager.addTrack(newAircraft);
            this.aircraft.push(newAircraft);

            const data = newAircraft.getTrackData();
            const msg = `NEW CONTACT: ${getTrackIdString(data.trackId)} ` +
                        `AZ:${Math.floor(data.azimuth)} RNG:${Math.floor(data.range)}km`;
            if (this.gameHUD) this.gameHUD.addMessage(msg);
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
            let msg = '';

            if (result.result === EngagementResult.HIT) {
                msg = `${result.batteryDesignation} -> ${aircraft ? getTrackIdString(result.trackId) : 'TK-???'} ** SPLASH **`;
                if (aircraft) {
                    if (aircraft.isFriendly()) {
                        this.onFriendlyDestroyed(aircraft);
                    } else {
                        this.onHostileDestroyed(aircraft);
                    }
                }
            } else if (result.result === EngagementResult.MISS) {
                msg = `${result.batteryDesignation} -> MISS`;
                this.addScore(GameConstants.SCORE_MISSILE_WASTED);
            }

            if (msg && this.gameHUD) {
                this.gameHUD.addMessage(msg);
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
            if (this.gameHUD) {
                this.gameHUD.addMessage('=== GAME OVER - DEFENSE FAILED ===');
            }
        }
    }

    // Input handlers
    private onTrackSelected(trackId: number): void {
        if (this.radarDisplay) this.radarDisplay.setSelectedTrack(trackId);
        if (this.gameHUD) this.gameHUD.setSelectedTrack(trackId);
    }

    private onBatteryAssign(batteryDesignation: string): void {
        if (!this.radarDisplay || this.radarDisplay.getSelectedTrack() < 0) return;

        const battery = this.fireControlSystem.getBattery(batteryDesignation);
        const aircraft = this.trackManager.getAircraftByTrackId(
            this.radarDisplay.getSelectedTrack());

        if (battery && aircraft && battery.canEngage(aircraft)) {
            if (battery.engage(aircraft)) {
                const msg = `${batteryDesignation} ENGAGING ${getTrackIdString(aircraft.getTrackId())}`;
                if (this.gameHUD) this.gameHUD.addMessage(msg);
            }
        }
    }

    private onFireAuthorized(): void {}
    private onAbortEngagement(): void {}

    // Scoring
    private addScore(points: number): void {
        this.score += points;
        if (this.gameHUD) this.gameHUD.setScore(this.score);
    }

    private onHostileDestroyed(aircraft: Aircraft): void {
        const score = this.gameConfig.getHostileDestroyedScore(aircraft.getType());
        this.addScore(score);
    }

    private onFriendlyDestroyed(_aircraft: Aircraft): void {
        this.addScore(GameConstants.SCORE_FRIENDLY_DESTROYED);
        if (this.gameHUD) {
            this.gameHUD.addMessage('*** WARNING: FRIENDLY FIRE ***');
        }
    }

    private onHostilePenetrated(_aircraft: Aircraft): void {
        this.addScore(GameConstants.SCORE_HOSTILE_PENETRATED);
        if (this.gameHUD) {
            this.gameHUD.addMessage('ALERT: HOSTILE PENETRATED DEFENSE ZONE');
        }
    }

    onDestroy(): void {
        input.off(Input.EventType.KEY_DOWN, this.onKeyDown, this);
    }
}
