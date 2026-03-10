import { _decorator, Component, Graphics, Color } from 'cc';
import {
    GameConstants, BatteryStatus,
    getTrackIdString, getClassificationString, getAltitudeString,
} from '../game/GameTypes';
import { TrackManager } from '../game/TrackManager';
import { FireControlSystem } from '../game/FireControlSystem';
import { ThreatBoard } from '../game/ThreatBoard';
import { BattalionHQ } from '../game/BattalionHQ';

const { ccclass, property } = _decorator;

// ============================================================================
// OverheadHUD — the red LED alphanumeric status displays mounted above
// the PPI scopes on the AN/TSQ-73 console (Item 12: Status Display Panel).
//
// These are the two rectangular LED matrix panels visible in the reference
// photo, showing track tables, battery status, and engagement data in
// red/amber segmented characters on a dark background.
// ============================================================================

@ccclass('OverheadHUD')
export class OverheadHUD extends Component {
    @property
    displayWidth: number = 380;

    @property
    displayHeight: number = 200;

    @property
    spacing: number = 30;

    private graphics: Graphics | null = null;

    private trackManager: TrackManager | null = null;
    private fireControl: FireControlSystem | null = null;
    private threatBoard: ThreatBoard | null = null;
    private battalionHQ: BattalionHQ | null = null;
    private score: number = 0;
    private level: number = 1;

    // Character grid dimensions for LED simulation
    private readonly CHAR_W = 7;
    private readonly CHAR_H = 10;
    private readonly LINE_H = 13;

    start(): void {
        this.graphics = this.getComponent(Graphics);
        if (!this.graphics) {
            this.graphics = this.addComponent(Graphics);
        }
    }

    setTrackManager(mgr: TrackManager): void { this.trackManager = mgr; }
    setFireControlSystem(fcs: FireControlSystem): void { this.fireControl = fcs; }
    setThreatBoard(tb: ThreatBoard): void { this.threatBoard = tb; }
    setBattalionHQ(hq: BattalionHQ): void { this.battalionHQ = hq; }
    setScore(s: number): void { this.score = s; }
    setLevel(l: number): void { this.level = l; }

    update(dt: number): void {
        if (!this.graphics) return;
        this.graphics.clear();

        this.drawDisplayHousing();
        this.drawLeftDisplay();
        this.drawRightDisplay();
    }

    private drawDisplayHousing(): void {
        const g = this.graphics!;
        const totalW = this.displayWidth * 2 + this.spacing;
        const startX = -totalW / 2;

        // Mounting bracket — metal bar above the displays
        g.fillColor = new Color(50, 55, 45, 255);
        g.roundRect(startX - 20, this.displayHeight + 5, totalW + 40, 12, 2);
        g.fill();
        g.strokeColor = new Color(70, 75, 62, 255);
        g.lineWidth = 1;
        g.roundRect(startX - 20, this.displayHeight + 5, totalW + 40, 12, 2);
        g.stroke();

        // Left display housing
        this.drawDisplayCase(startX, 0);
        // Right display housing
        this.drawDisplayCase(startX + this.displayWidth + this.spacing, 0);
    }

    private drawDisplayCase(x: number, y: number): void {
        const g = this.graphics!;
        const w = this.displayWidth;
        const h = this.displayHeight;

        // Outer case — dark metal
        g.fillColor = new Color(25, 25, 22, 255);
        g.roundRect(x - 6, y - 6, w + 12, h + 12, 4);
        g.fill();
        g.strokeColor = new Color(60, 65, 52, 255);
        g.lineWidth = 1.5;
        g.roundRect(x - 6, y - 6, w + 12, h + 12, 4);
        g.stroke();

        // Display screen — very dark with slight reddish tint
        g.fillColor = new Color(15, 5, 5, 255);
        g.roundRect(x, y, w, h, 2);
        g.fill();

        // Screen inner bevel
        g.strokeColor = new Color(40, 15, 15, 255);
        g.lineWidth = 1;
        g.roundRect(x + 1, y + 1, w - 2, h - 2, 2);
        g.stroke();
    }

    private drawLeftDisplay(): void {
        // Left display: TRACK TABLE + SCORE
        const totalW = this.displayWidth * 2 + this.spacing;
        const startX = -totalW / 2;
        const x = startX + 8;
        const topY = this.displayHeight - 14;

        // Header
        this.drawLEDText(x, topY, 'TRACK TABLE', new Color(255, 60, 60, 255));
        this.drawLEDText(x + 260, topY, `LV:${this.level}`, new Color(255, 180, 60, 255));

        // Column headers
        const headerY = topY - this.LINE_H;
        const dimRed = new Color(180, 40, 40, 200);
        this.drawLEDText(x, headerY, 'TRK', dimRed);
        this.drawLEDText(x + 50, headerY, 'CLS', dimRed);
        this.drawLEDText(x + 100, headerY, 'ALT', dimRed);
        this.drawLEDText(x + 155, headerY, 'AZM', dimRed);
        this.drawLEDText(x + 205, headerY, 'RNG', dimRed);
        this.drawLEDText(x + 260, headerY, 'SPD', dimRed);
        this.drawLEDText(x + 310, headerY, 'HDG', dimRed);

        // Separator line
        const g = this.graphics!;
        g.strokeColor = new Color(120, 30, 30, 150);
        g.lineWidth = 1;
        g.moveTo(x, headerY - 3);
        g.lineTo(x + this.displayWidth - 16, headerY - 3);
        g.stroke();

        // Track rows
        if (this.trackManager) {
            const tracks = this.trackManager.getAllTracks();
            const maxRows = 12;
            for (let i = 0; i < Math.min(tracks.length, maxRows); i++) {
                const track = tracks[i];
                const rowY = headerY - this.LINE_H * (i + 1) - 2;

                // Color based on classification
                let rowColor: Color;
                switch (track.classification) {
                    case 2: rowColor = new Color(255, 60, 60, 255); break;   // HOSTILE - bright red
                    case 1: rowColor = new Color(60, 120, 255, 220); break;  // FRIENDLY - blue
                    case 3: rowColor = new Color(255, 180, 60, 220); break;  // UNKNOWN - amber
                    default: rowColor = new Color(180, 180, 180, 180); break; // PENDING - gray
                }

                this.drawLEDText(x, rowY, getTrackIdString(track.trackId), rowColor);
                this.drawLEDText(x + 50, rowY, getClassificationString(track.classification).substring(0, 4), rowColor);
                this.drawLEDText(x + 100, rowY, getAltitudeString(track.altitude), rowColor);
                this.drawLEDText(x + 155, rowY, String(Math.floor(track.azimuth)).padStart(3, '0'), rowColor);
                this.drawLEDText(x + 205, rowY,
                    (track.range * GameConstants.KM_TO_NM).toFixed(0), rowColor);
                this.drawLEDText(x + 260, rowY, String(Math.floor(track.speed)), rowColor);
                this.drawLEDText(x + 310, rowY, String(Math.floor(track.heading)).padStart(3, '0'), rowColor);
            }
        }

        // Score at bottom
        const scoreY = 8;
        this.drawLEDText(x, scoreY, `SCORE: ${this.score}`, new Color(255, 100, 60, 255));
    }

    private drawRightDisplay(): void {
        // Right display: BATTERY STATUS + THREAT BOARD
        const totalW = this.displayWidth * 2 + this.spacing;
        const startX = -totalW / 2 + this.displayWidth + this.spacing;
        const x = startX + 8;
        const topY = this.displayHeight - 14;

        // Battery status section
        this.drawLEDText(x, topY, 'BATTERY STATUS', new Color(255, 60, 60, 255));

        if (this.fireControl) {
            const batteries = this.fireControl.getAllBatteryData();
            const dimRed = new Color(180, 40, 40, 200);

            // Column headers
            const hdrY = topY - this.LINE_H;
            this.drawLEDText(x, hdrY, 'UNIT', dimRed);
            this.drawLEDText(x + 95, hdrY, 'MSL', dimRed);
            this.drawLEDText(x + 140, hdrY, 'STS', dimRed);
            this.drawLEDText(x + 195, hdrY, 'TGT', dimRed);

            const g = this.graphics!;
            g.strokeColor = new Color(120, 30, 30, 150);
            g.lineWidth = 1;
            g.moveTo(x, hdrY - 3);
            g.lineTo(x + this.displayWidth - 16, hdrY - 3);
            g.stroke();

            for (let i = 0; i < batteries.length; i++) {
                const bat = batteries[i];
                const rowY = hdrY - this.LINE_H * (i + 1) - 2;

                let statusStr: string;
                let statusColor: Color;
                switch (bat.status) {
                    case BatteryStatus.READY:
                        statusStr = 'RDY';
                        statusColor = new Color(60, 255, 60, 255);
                        break;
                    case BatteryStatus.ENGAGED:
                        statusStr = 'ENG';
                        statusColor = new Color(255, 60, 60, 255);
                        break;
                    case BatteryStatus.RELOADING:
                        statusStr = 'RLD';
                        statusColor = new Color(255, 180, 60, 255);
                        break;
                    case BatteryStatus.OFFLINE:
                        statusStr = 'OFF';
                        statusColor = new Color(120, 120, 120, 200);
                        break;
                    case BatteryStatus.DESTROYED:
                        statusStr = 'DES';
                        statusColor = new Color(255, 0, 0, 255);
                        break;
                    default:
                        statusStr = 'TRK';
                        statusColor = new Color(255, 255, 60, 255);
                        break;
                }

                const unitColor = new Color(220, 80, 60, 240);
                this.drawLEDText(x, rowY, bat.designation, unitColor);
                this.drawLEDText(x + 95, rowY, `${bat.missilesRemaining}/${bat.maxMissiles}`, unitColor);
                this.drawLEDText(x + 140, rowY, statusStr, statusColor);

                const tgtStr = bat.assignedTrackId >= 0 ? getTrackIdString(bat.assignedTrackId) : '---';
                this.drawLEDText(x + 195, rowY, tgtStr, unitColor);
            }
        }

        // HQ Status at bottom
        if (this.battalionHQ) {
            const hqData = this.battalionHQ.getData();
            const hqY = 22;
            const hqColor = hqData.radarOnline
                ? new Color(60, 255, 60, 230)
                : new Color(255, 60, 60, 230);
            this.drawLEDText(x, hqY,
                `HQ:${hqData.statusString} RDR:${hqData.radarOnline ? 'ON' : 'OFF'}`,
                hqColor);
        }

        // Threat count
        if (this.threatBoard) {
            const threatColor = new Color(255, 100, 60, 255);
            this.drawLEDText(x + 240, 22,
                `THR:${this.threatBoard.getThreatCount()}`,
                threatColor);
        }
    }

    // Simulate LED segmented text by drawing small rectangles for each character
    private drawLEDText(x: number, y: number, text: string, color: Color): void {
        const g = this.graphics!;

        // Each character is a small illuminated block — simulates LED dot matrix
        for (let i = 0; i < text.length; i++) {
            const ch = text[i];
            const cx = x + i * this.CHAR_W;

            if (ch === ' ') continue;

            // Character glow (subtle bloom behind text)
            g.fillColor = new Color(color.r, color.g, color.b, Math.floor(color.a * 0.15));
            g.roundRect(cx - 1, y - 1, this.CHAR_W, this.CHAR_H + 2, 1);
            g.fill();

            // Character body — small filled rect per character
            // This simulates the blocky LED segment look
            g.fillColor = color;
            g.roundRect(cx, y, this.CHAR_W - 2, this.CHAR_H, 0.5);
            g.fill();
        }
    }
}
