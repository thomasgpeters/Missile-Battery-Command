import { _decorator, Component, Node, Graphics, Color, input, Input, EventKeyboard, KeyCode, EventTouch, UITransform, Vec3 } from 'cc';
import {
    GameConstants, EngagementResult, BatteryStatus,
    getTrackIdString, getClassificationString, getAltitudeString,
} from '../game/GameTypes';
import { Aircraft } from '../game/Aircraft';
import { AircraftGenerator } from '../game/AircraftGenerator';
import { TrackManager } from '../game/TrackManager';
import { FireControlSystem } from '../game/FireControlSystem';
import { GameConfig } from '../game/GameConfig';
import { ThreatBoard } from '../game/ThreatBoard';
import { BattalionHQ } from '../game/BattalionHQ';
import { RadarDisplay } from './RadarDisplay';
import { ConsoleFrame } from './ConsoleFrame';

const { ccclass, property } = _decorator;

// ============================================================================
// IntegratedConsoleScene — single AN/TSQ-73 console with all information
// combined into one view.
//
// Layout:
//   - Center: PPI radar scope (RadarDisplay)
//   - Console housing with scope bezel, knobs, etc (ConsoleFrame)
//   - Left panel:  Live track table (LED-style text drawn over the buttons)
//   - Right panel: Battery status + HQ/threat info (LED-style text)
//   - Bottom strip: Score, level, and message log
//
// This is the simplest scene to set up in Cocos Creator — only needs:
//   1. A root node with IntegratedConsoleScene
//   2. A child node with RadarDisplay + Graphics (centered)
//   3. A child node with ConsoleFrame + Graphics (centered, same position)
//   4. A child node with Graphics for the data overlay panels
//
// All track data, battery status, threat board, HQ status, and score are
// rendered directly into the console's side panel areas using Graphics
// primitive drawing — no Label nodes needed.
// ============================================================================

@ccclass('IntegratedConsoleScene')
export class IntegratedConsoleScene extends Component {
    @property(RadarDisplay)
    radarDisplay: RadarDisplay | null = null;

    @property(ConsoleFrame)
    consoleFrame: ConsoleFrame | null = null;

    // Graphics component for drawing the data overlay panels
    // (track table, battery status, score, messages)
    @property(Graphics)
    dataOverlay: Graphics | null = null;

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
    private readonly MAX_MESSAGES = 6;

    // LED text dimensions
    private readonly CHAR_W = 6;
    private readonly CHAR_H = 9;
    private readonly LINE_H = 12;

    start(): void {
        this.gameConfig.setLevel(1);
        this.aircraftGenerator.init(this.gameConfig.getLevel());
        this.fireControlSystem.init();
        this.battalionHQ.init(0.5, 0.0);
        this.trackManager.getIFFSystem().setErrorRate(this.gameConfig.getIFFErrorRate());

        // Wire up radar display
        if (this.radarDisplay) {
            this.radarDisplay.setTrackManager(this.trackManager);
            this.radarDisplay.setFireControlSystem(this.fireControlSystem);
        }

        // Ensure ConsoleFrame doesn't draw over static buttons since
        // we'll overlay live data on the side panels
        if (this.consoleFrame) {
            this.consoleFrame.drawBackground = true;
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

        // Draw the data overlay panels every frame
        this.drawDataPanels();
    }

    // ========================================================================
    // Data Overlay Rendering — LED-style text in the console's side panels
    // ========================================================================

    private drawDataPanels(): void {
        if (!this.dataOverlay) return;
        const g = this.dataOverlay;
        g.clear();

        const scopeR = this.consoleFrame ? this.consoleFrame.scopeRadius : 280;
        const panelW = this.consoleFrame ? this.consoleFrame.panelWidth : 160;

        this.drawLeftPanel(g, scopeR, panelW);
        this.drawRightPanel(g, scopeR, panelW);
        this.drawBottomBar(g, scopeR, panelW);
    }

    // Left panel: Track table
    private drawLeftPanel(g: Graphics, scopeR: number, panelW: number): void {
        const panelX = -scopeR - panelW - 16;
        const panelTop = scopeR - 26;

        // Panel backing — dark overlay to cover the static buttons
        g.fillColor = new Color(20, 22, 18, 240);
        g.roundRect(panelX + 4, -scopeR + 24, panelW - 8, scopeR * 2 - 48, 2);
        g.fill();

        // Header
        this.drawLED(g, panelX + 8, panelTop, 'TRACK TABLE', new Color(255, 60, 60, 255));

        // Column headers
        const hdrY = panelTop - this.LINE_H;
        const dim = new Color(180, 40, 40, 200);
        this.drawLED(g, panelX + 8, hdrY, 'TRK', dim);
        this.drawLED(g, panelX + 38, hdrY, 'CLS', dim);
        this.drawLED(g, panelX + 68, hdrY, 'RNG', dim);
        this.drawLED(g, panelX + 98, hdrY, 'AZM', dim);
        this.drawLED(g, panelX + 128, hdrY, 'ALT', dim);

        // Separator
        g.strokeColor = new Color(120, 30, 30, 150);
        g.lineWidth = 1;
        g.moveTo(panelX + 8, hdrY - 3);
        g.lineTo(panelX + panelW - 12, hdrY - 3);
        g.stroke();

        // Track rows
        const tracks = this.trackManager.getAllTracks();
        const maxRows = Math.min(tracks.length, 18);
        for (let i = 0; i < maxRows; i++) {
            const track = tracks[i];
            if (!track.isAlive) continue;
            const rowY = hdrY - this.LINE_H * (i + 1) - 2;

            let rowColor: Color;
            switch (track.classification) {
                case 2: rowColor = new Color(255, 60, 60, 255); break;   // HOSTILE
                case 1: rowColor = new Color(60, 120, 255, 220); break;  // FRIENDLY
                case 3: rowColor = new Color(255, 180, 60, 220); break;  // UNKNOWN
                default: rowColor = new Color(180, 180, 180, 180); break; // PENDING
            }

            // Highlight selected track
            if (track.trackId === (this.radarDisplay?.getSelectedTrack() ?? -1)) {
                g.fillColor = new Color(255, 255, 0, 30);
                g.roundRect(panelX + 6, rowY - 1, panelW - 12, this.LINE_H, 1);
                g.fill();
                rowColor = new Color(255, 255, 60, 255);
            }

            this.drawLED(g, panelX + 8, rowY, getTrackIdString(track.trackId), rowColor);
            this.drawLED(g, panelX + 38, rowY,
                getClassificationString(track.classification).substring(0, 4), rowColor);
            this.drawLED(g, panelX + 68, rowY,
                (track.range * GameConstants.KM_TO_NM).toFixed(0), rowColor);
            this.drawLED(g, panelX + 98, rowY,
                String(Math.floor(track.azimuth)).padStart(3, '0'), rowColor);
            this.drawLED(g, panelX + 128, rowY,
                getAltitudeString(track.altitude), rowColor);
        }
    }

    // Right panel: Battery status + HQ + Threat
    private drawRightPanel(g: Graphics, scopeR: number, panelW: number): void {
        const panelX = scopeR + 24;
        const panelTop = scopeR - 26;

        // Panel backing
        g.fillColor = new Color(20, 22, 18, 240);
        g.roundRect(panelX - 4, -scopeR + 24, panelW - 8, scopeR * 2 - 48, 2);
        g.fill();

        // Battery status header
        this.drawLED(g, panelX, panelTop, 'BATTERY STATUS', new Color(255, 60, 60, 255));

        const hdrY = panelTop - this.LINE_H;
        const dim = new Color(180, 40, 40, 200);
        this.drawLED(g, panelX, hdrY, 'UNIT', dim);
        this.drawLED(g, panelX + 65, hdrY, 'MSL', dim);
        this.drawLED(g, panelX + 95, hdrY, 'STS', dim);
        this.drawLED(g, panelX + 125, hdrY, 'TGT', dim);

        g.strokeColor = new Color(120, 30, 30, 150);
        g.lineWidth = 1;
        g.moveTo(panelX, hdrY - 3);
        g.lineTo(panelX + panelW - 20, hdrY - 3);
        g.stroke();

        // Battery rows
        const batteries = this.fireControlSystem.getAllBatteryData();
        for (let i = 0; i < batteries.length; i++) {
            const bat = batteries[i];
            const rowY = hdrY - this.LINE_H * (i + 1) - 2;

            let statusStr: string;
            let statusColor: Color;
            switch (bat.status) {
                case BatteryStatus.READY:
                    statusStr = 'RDY'; statusColor = new Color(60, 255, 60, 255); break;
                case BatteryStatus.ENGAGED:
                    statusStr = 'ENG'; statusColor = new Color(255, 60, 60, 255); break;
                case BatteryStatus.RELOADING:
                    statusStr = 'RLD'; statusColor = new Color(255, 180, 60, 255); break;
                case BatteryStatus.OFFLINE:
                    statusStr = 'OFF'; statusColor = new Color(120, 120, 120, 200); break;
                case BatteryStatus.DESTROYED:
                    statusStr = 'DES'; statusColor = new Color(255, 0, 0, 255); break;
                default:
                    statusStr = 'TRK'; statusColor = new Color(255, 255, 60, 255); break;
            }

            const unitColor = new Color(220, 80, 60, 240);
            this.drawLED(g, panelX, rowY, bat.designation, unitColor);
            this.drawLED(g, panelX + 65, rowY,
                `${bat.missilesRemaining}/${bat.maxMissiles}`, unitColor);
            this.drawLED(g, panelX + 95, rowY, statusStr, statusColor);

            const tgtStr = bat.assignedTrackId >= 0
                ? getTrackIdString(bat.assignedTrackId) : '---';
            this.drawLED(g, panelX + 125, rowY, tgtStr, unitColor);
        }

        // HQ Status section — below batteries
        const hqSectionY = hdrY - this.LINE_H * (batteries.length + 2);

        g.strokeColor = new Color(120, 30, 30, 100);
        g.lineWidth = 1;
        g.moveTo(panelX, hqSectionY + this.LINE_H + 2);
        g.lineTo(panelX + panelW - 20, hqSectionY + this.LINE_H + 2);
        g.stroke();

        this.drawLED(g, panelX, hqSectionY + this.LINE_H - 2,
            'HQ / THREAT', new Color(255, 60, 60, 255));

        const hqData = this.battalionHQ.getData();
        const hqColor = hqData.radarOnline
            ? new Color(60, 255, 60, 230)
            : new Color(255, 60, 60, 230);
        this.drawLED(g, panelX, hqSectionY - 2,
            `HQ:${hqData.statusString}`, hqColor);
        this.drawLED(g, panelX, hqSectionY - this.LINE_H - 2,
            `RDR:${hqData.radarOnline ? 'ON' : 'OFF'} COM:${hqData.commsOnline ? 'ON' : 'OFF'}`,
            hqColor);

        if (hqData.isRelocating) {
            this.drawLED(g, panelX, hqSectionY - this.LINE_H * 2 - 2,
                `ETA:${Math.floor(hqData.relocateTimeRemaining)}s`,
                new Color(255, 180, 60, 255));
        }

        // Threat count
        const threatY = hqSectionY - this.LINE_H * 3 - 2;
        const threatCount = this.threatBoard.getThreatCount();
        const threatColor = threatCount > 0
            ? new Color(255, 60, 60, 255)
            : new Color(60, 255, 60, 200);
        this.drawLED(g, panelX, threatY,
            `THREATS: ${threatCount}`, threatColor);
    }

    // Bottom bar: Score, level, messages
    private drawBottomBar(g: Graphics, scopeR: number, panelW: number): void {
        const barX = -scopeR - panelW - 16;
        const barW = (scopeR + panelW + 20) * 2;
        const barY = -scopeR - 68;

        // Background overlay
        g.fillColor = new Color(20, 22, 18, 220);
        g.roundRect(barX + 4, barY - 30, barW - 8, 28, 2);
        g.fill();

        // Score and level on left
        this.drawLED(g, barX + 10, barY - 22,
            `SCORE: ${this.score}`, new Color(255, 100, 60, 255));
        this.drawLED(g, barX + 120, barY - 22,
            `LV:${this.gameConfig.getLevel()}`, new Color(255, 180, 60, 255));

        // Most recent message on right
        if (this.messages.length > 0) {
            const msg = this.messages[this.messages.length - 1];
            this.drawLED(g, barX + 200, barY - 22,
                msg.substring(0, 50), new Color(0, 220, 0, 200));
        }

        if (this.gameOver) {
            this.drawLED(g, barX + barW / 2 - 80, barY - 10,
                '=== GAME OVER ===', new Color(255, 0, 0, 255));
        }
    }

    // LED-style character rendering (same approach as OverheadHUD)
    private drawLED(g: Graphics, x: number, y: number, text: string, color: Color): void {
        for (let i = 0; i < text.length; i++) {
            if (text[i] === ' ') continue;
            const cx = x + i * this.CHAR_W;

            // Glow
            g.fillColor = new Color(color.r, color.g, color.b, Math.floor(color.a * 0.15));
            g.roundRect(cx - 1, y - 1, this.CHAR_W, this.CHAR_H + 2, 1);
            g.fill();

            // Character block
            g.fillColor = color;
            g.roundRect(cx, y, this.CHAR_W - 2, this.CHAR_H, 0.5);
            g.fill();
        }
    }

    // ========================================================================
    // Game Logic
    // ========================================================================

    private spawnAircraft(dt: number): void {
        const currentCount = this.aircraft.filter((ac) => ac.isAlive()).length;
        const newAircraft = this.aircraftGenerator.trySpawn(dt, currentCount);
        if (newAircraft) {
            this.trackManager.addTrack(newAircraft);
            this.aircraft.push(newAircraft);

            const data = newAircraft.getTrackData();
            this.addMessage(
                `NEW: ${getTrackIdString(data.trackId)} AZ:${Math.floor(data.azimuth)} RNG:${Math.floor(data.range)}km`
            );
        }
    }

    private updateAircraft(dt: number): void {
        for (const ac of this.aircraft) {
            if (ac.isAlive()) {
                ac.update(dt);
                if (ac.hasReachedTerritory() && !ac.isFriendly()) {
                    this.score += GameConstants.SCORE_HOSTILE_PENETRATED;
                    this.addMessage('ALERT: HOSTILE PENETRATED DEFENSE ZONE');
                    ac.destroy();
                }
            }
        }
    }

    private checkEngagements(): void {
        for (const result of this.fireControlSystem.getRecentResults()) {
            const aircraft = this.trackManager.getAircraftByTrackId(result.trackId);
            if (result.result === EngagementResult.HIT && aircraft) {
                if (aircraft.isFriendly()) {
                    this.score += GameConstants.SCORE_FRIENDLY_DESTROYED;
                    this.addMessage('*** WARNING: FRIENDLY FIRE ***');
                } else {
                    this.score += this.gameConfig.getHostileDestroyedScore(aircraft.getType());
                    this.addMessage(
                        `${result.batteryDesignation} -> ${getTrackIdString(result.trackId)} ** SPLASH **`
                    );
                }
            } else if (result.result === EngagementResult.MISS) {
                this.score += GameConstants.SCORE_MISSILE_WASTED;
                this.addMessage(`${result.batteryDesignation} -> MISS`);
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
            this.addMessage('=== GAME OVER - DEFENSE FAILED ===');
        }
    }

    private addMessage(msg: string): void {
        this.messages.push(msg);
        if (this.messages.length > this.MAX_MESSAGES) {
            this.messages.shift();
        }
    }

    // ========================================================================
    // Input Handlers
    // ========================================================================

    private onTrackSelected(trackId: number): void {
        if (this.radarDisplay) this.radarDisplay.setSelectedTrack(trackId);
    }

    private onBatteryAssign(designation: string): void {
        if (!this.radarDisplay || this.radarDisplay.getSelectedTrack() < 0) return;
        const battery = this.fireControlSystem.getBattery(designation);
        const aircraft = this.trackManager.getAircraftByTrackId(
            this.radarDisplay.getSelectedTrack());
        if (battery && aircraft && battery.canEngage(aircraft)) {
            if (battery.engage(aircraft)) {
                this.addMessage(
                    `${designation} ENGAGING ${getTrackIdString(aircraft.getTrackId())}`
                );
            }
        }
    }

    private onFireAuthorized(): void {}
    private onAbortEngagement(): void {}

    onDestroy(): void {
        input.off(Input.EventType.KEY_DOWN, this.onKeyDown, this);
    }
}
