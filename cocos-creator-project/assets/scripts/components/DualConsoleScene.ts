import { _decorator, Component, Node, input, Input, EventKeyboard, KeyCode, EventTouch, UITransform, Vec3, Graphics, Color } from 'cc';
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
// DualConsoleScene — wide-angle view of both operator stations in the
// AN/TSQ-73 S-280 shelter, as seen in the reference photo.
//
// Layout:
//   - Two consoles side by side (items 11 & 13 from the shelter diagram)
//   - Shared overhead HUD panels above (item 12: Status Display Panel)
//   - Center control panel between the two consoles
//   - Dark shelter interior background
//   - Player controls the left console; right console shows AI/second scope
//
// This view is for the cinematic "wide shot" of the Missile Minder battery.
// ============================================================================

@ccclass('DualConsoleScene')
export class DualConsoleScene extends Component {
    // Left console (player-controlled)
    @property(RadarDisplay)
    leftRadarDisplay: RadarDisplay | null = null;

    @property(ConsoleFrame)
    leftConsoleFrame: ConsoleFrame | null = null;

    // Right console (second operator / AI)
    @property(RadarDisplay)
    rightRadarDisplay: RadarDisplay | null = null;

    @property(ConsoleFrame)
    rightConsoleFrame: ConsoleFrame | null = null;

    // Shared overhead displays
    @property(OverheadHUD)
    overheadHUD: OverheadHUD | null = null;

    // Shelter interior background
    @property(Graphics)
    shelterGraphics: Graphics | null = null;

    // Core game systems (shared between both consoles)
    private aircraftGenerator = new AircraftGenerator();
    private trackManager = new TrackManager();
    private fireControlSystem = new FireControlSystem();
    private threatBoard = new ThreatBoard();
    private battalionHQ = new BattalionHQ();
    private gameConfig = GameConfig.getInstance();

    private score: number = 0;
    private gameOver: boolean = false;
    private aircraft: Aircraft[] = [];

    start(): void {
        this.gameConfig.setLevel(1);
        this.aircraftGenerator.init(this.gameConfig.getLevel());
        this.fireControlSystem.init();
        this.battalionHQ.init(0.5, 0.0);
        this.trackManager.getIFFSystem().setErrorRate(this.gameConfig.getIFFErrorRate());

        // Both consoles share the same track and fire control data
        if (this.leftRadarDisplay) {
            this.leftRadarDisplay.setTrackManager(this.trackManager);
            this.leftRadarDisplay.setFireControlSystem(this.fireControlSystem);
        }
        if (this.rightRadarDisplay) {
            this.rightRadarDisplay.setTrackManager(this.trackManager);
            this.rightRadarDisplay.setFireControlSystem(this.fireControlSystem);
        }

        if (this.overheadHUD) {
            this.overheadHUD.setTrackManager(this.trackManager);
            this.overheadHUD.setFireControlSystem(this.fireControlSystem);
            this.overheadHUD.setThreatBoard(this.threatBoard);
            this.overheadHUD.setBattalionHQ(this.battalionHQ);
            this.overheadHUD.setLevel(this.gameConfig.getLevel());
        }

        this.drawShelterInterior();
        this.initInputHandlers();
    }

    private drawShelterInterior(): void {
        if (!this.shelterGraphics) return;
        const g = this.shelterGraphics;
        g.clear();

        // S-280 shelter interior — dark olive/gray walls
        g.fillColor = new Color(18, 20, 16, 255);
        g.rect(-960, -540, 1920, 1080);
        g.fill();

        // Ceiling with overhead lighting strips (dim)
        g.fillColor = new Color(22, 25, 20, 255);
        g.rect(-960, 300, 1920, 240);
        g.fill();

        // Overhead light strips (very dim red for night ops)
        for (let lx = -700; lx <= 700; lx += 350) {
            g.fillColor = new Color(60, 15, 15, 120);
            g.roundRect(lx - 40, 480, 80, 8, 2);
            g.fill();
            // Light glow
            g.fillColor = new Color(40, 10, 10, 40);
            g.roundRect(lx - 60, 460, 120, 40, 4);
            g.fill();
        }

        // Floor (darker)
        g.fillColor = new Color(12, 14, 10, 255);
        g.rect(-960, -540, 1920, 160);
        g.fill();

        // Center divider panel between the two consoles
        // This is the shared control panel / equipment rack
        g.fillColor = new Color(35, 38, 32, 255);
        g.roundRect(-30, -350, 60, 600, 4);
        g.fill();
        g.strokeColor = new Color(50, 55, 45, 255);
        g.lineWidth = 1;
        g.roundRect(-30, -350, 60, 600, 4);
        g.stroke();

        // Small indicator lights on center panel
        for (let cy = -200; cy <= 200; cy += 40) {
            const litColor = Math.random() > 0.3
                ? new Color(0, 180, 0, 200)
                : new Color(180, 0, 0, 180);
            g.fillColor = litColor;
            g.circle(0, cy, 3);
            g.fill();
        }

        // Cable runs along the floor
        g.strokeColor = new Color(30, 35, 28, 200);
        g.lineWidth = 4;
        g.moveTo(-300, -380);
        g.bezierCurveTo(-200, -400, -100, -390, 0, -380);
        g.bezierCurveTo(100, -370, 200, -390, 300, -380);
        g.stroke();
    }

    private initInputHandlers(): void {
        if (this.leftRadarDisplay) {
            this.leftRadarDisplay.node.on(Node.EventType.TOUCH_END, this.onRadarTouch, this);
        }
        input.on(Input.EventType.KEY_DOWN, this.onKeyDown, this);
    }

    private onRadarTouch(event: EventTouch): void {
        if (!this.leftRadarDisplay) return;
        const uiPos = event.getUILocation();
        const transform = this.leftRadarDisplay.node.getComponent(UITransform);
        if (!transform) return;
        const nodePos = transform.convertToNodeSpaceAR(new Vec3(uiPos.x, uiPos.y, 0));
        const nearestTrackId = this.leftRadarDisplay.findNearestTrack(nodePos.x, nodePos.y);
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

        if (this.overheadHUD) this.overheadHUD.setScore(this.score);
        if (this.score < -2000) this.gameOver = true;
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
                    this.score += GameConstants.SCORE_HOSTILE_PENETRATED;
                    ac.destroy();
                }
            }
        }
    }

    private checkEngagements(): void {
        for (const result of this.fireControlSystem.getRecentResults()) {
            const aircraft = this.trackManager.getAircraftByTrackId(result.trackId);
            if (result.result === EngagementResult.HIT && aircraft) {
                this.score += aircraft.isFriendly()
                    ? GameConstants.SCORE_FRIENDLY_DESTROYED
                    : this.gameConfig.getHostileDestroyedScore(aircraft.getType());
            } else if (result.result === EngagementResult.MISS) {
                this.score += GameConstants.SCORE_MISSILE_WASTED;
            }
        }
    }

    private cleanupDestroyedAircraft(): void {
        this.aircraft = this.aircraft.filter((ac) =>
            ac.isAlive() && ac.getRange() <= GameConstants.RADAR_MAX_RANGE_KM * 1.1);
    }

    private onTrackSelected(trackId: number): void {
        if (this.leftRadarDisplay) this.leftRadarDisplay.setSelectedTrack(trackId);
    }

    private onBatteryAssign(designation: string): void {
        if (!this.leftRadarDisplay || this.leftRadarDisplay.getSelectedTrack() < 0) return;
        const battery = this.fireControlSystem.getBattery(designation);
        const aircraft = this.trackManager.getAircraftByTrackId(
            this.leftRadarDisplay.getSelectedTrack());
        if (battery && aircraft && battery.canEngage(aircraft)) {
            battery.engage(aircraft);
        }
    }

    onDestroy(): void {
        input.off(Input.EventType.KEY_DOWN, this.onKeyDown, this);
    }
}
