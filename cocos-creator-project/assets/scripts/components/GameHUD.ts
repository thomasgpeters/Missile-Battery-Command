import { _decorator, Component, Label, Node, Color } from 'cc';
import {
    GameConstants, getTrackIdString, getClassificationString, getAltitudeString,
    BatteryStatus,
} from '../game/GameTypes';
import { TrackManager } from '../game/TrackManager';
import { FireControlSystem } from '../game/FireControlSystem';
import { ThreatBoard } from '../game/ThreatBoard';
import { BattalionHQ } from '../game/BattalionHQ';

const { ccclass, property } = _decorator;

// ============================================================================
// Game HUD - displays track info panel, battery status, and score
// ============================================================================

@ccclass('GameHUD')
export class GameHUD extends Component {
    @property(Label)
    trackInfoLabel: Label | null = null;

    @property(Label)
    batteryStatusLabel: Label | null = null;

    @property(Label)
    threatBoardLabel: Label | null = null;

    @property(Label)
    hqStatusLabel: Label | null = null;

    @property(Label)
    scoreLabel: Label | null = null;

    @property(Label)
    levelLabel: Label | null = null;

    @property(Label)
    messageLogLabel: Label | null = null;

    private trackManager: TrackManager | null = null;
    private fireControl: FireControlSystem | null = null;
    private threatBoard: ThreatBoard | null = null;
    private battalionHQ: BattalionHQ | null = null;

    private selectedTrackId: number = -1;
    private score: number = 0;
    private level: number = 1;
    private messages: string[] = [];
    private readonly MAX_MESSAGES = 8;

    setTrackManager(mgr: TrackManager): void { this.trackManager = mgr; }
    setFireControlSystem(fcs: FireControlSystem): void { this.fireControl = fcs; }
    setThreatBoard(tb: ThreatBoard): void { this.threatBoard = tb; }
    setBattalionHQ(hq: BattalionHQ): void { this.battalionHQ = hq; }

    setSelectedTrack(trackId: number): void { this.selectedTrackId = trackId; }
    setScore(score: number): void { this.score = score; }
    setLevel(level: number): void { this.level = level; }

    addMessage(message: string): void {
        this.messages.push(message);
        if (this.messages.length > this.MAX_MESSAGES) {
            this.messages.shift();
        }
    }

    update(dt: number): void {
        this.updateTrackInfoPanel();
        this.updateBatteryStatusPanel();
        this.updateThreatBoardPanel();
        this.updateHQStatusPanel();
        this.updateScoreDisplay();
        this.updateMessageLog();
    }

    private updateTrackInfoPanel(): void {
        if (!this.trackInfoLabel || !this.trackManager) return;

        if (this.selectedTrackId < 0) {
            this.trackInfoLabel.string = '=== TRACK INFO ===\nNo track selected';
            return;
        }

        const track = this.trackManager.getTrack(this.selectedTrackId);
        if (!track) {
            this.trackInfoLabel.string = '=== TRACK INFO ===\nTrack lost';
            return;
        }

        const lines = [
            '=== TRACK INFO ===',
            `TRACK: ${getTrackIdString(track.trackId)}`,
            `CLASS: ${getClassificationString(track.classification)}`,
            `ALT:   ${getAltitudeString(track.altitude)}`,
            `AZM:   ${String(Math.floor(track.azimuth)).padStart(3, '0')}°`,
            `RNG:   ${track.range.toFixed(1)} km (${(track.range * GameConstants.KM_TO_NM).toFixed(1)} NM)`,
            `SPD:   ${Math.floor(track.speed)} kts`,
            `HDG:   ${String(Math.floor(track.heading)).padStart(3, '0')}°`,
        ];
        this.trackInfoLabel.string = lines.join('\n');
    }

    private updateBatteryStatusPanel(): void {
        if (!this.batteryStatusLabel || !this.fireControl) return;

        const batteries = this.fireControl.getAllBatteryData();
        const lines = ['=== BATTERY STATUS ==='];

        for (const bat of batteries) {
            let statusStr: string;
            switch (bat.status) {
                case BatteryStatus.READY:     statusStr = 'RDY'; break;
                case BatteryStatus.ENGAGED:   statusStr = 'ENG'; break;
                case BatteryStatus.RELOADING: statusStr = 'RLD'; break;
                case BatteryStatus.OFFLINE:   statusStr = 'OFF'; break;
                case BatteryStatus.DESTROYED: statusStr = 'DES'; break;
                default: statusStr = 'TRK'; break;
            }

            lines.push(`${bat.designation} ${bat.missilesRemaining}/${bat.maxMissiles} [${statusStr}]`);
        }

        this.batteryStatusLabel.string = lines.join('\n');
    }

    private updateThreatBoardPanel(): void {
        if (!this.threatBoardLabel || !this.threatBoard) return;
        this.threatBoardLabel.string = this.threatBoard.formatBoard();
    }

    private updateHQStatusPanel(): void {
        if (!this.hqStatusLabel || !this.battalionHQ) return;

        const data = this.battalionHQ.getData();
        const lines = [
            '=== BATTALION HQ ===',
            `STATUS: ${data.statusString}`,
            `RADAR:  ${data.radarOnline ? 'ONLINE' : 'OFFLINE'}`,
            `COMMS:  ${data.commsOnline ? 'ONLINE' : 'OFFLINE'}`,
        ];

        if (data.isRelocating) {
            lines.push(`ETA:    ${Math.floor(data.relocateTimeRemaining)}s`);
        }

        this.hqStatusLabel.string = lines.join('\n');
    }

    private updateScoreDisplay(): void {
        if (this.scoreLabel) {
            this.scoreLabel.string = `SCORE: ${this.score}`;
        }
        if (this.levelLabel) {
            this.levelLabel.string = `LEVEL: ${this.level}`;
        }
    }

    private updateMessageLog(): void {
        if (!this.messageLogLabel) return;
        this.messageLogLabel.string = this.messages.join('\n');
    }
}
