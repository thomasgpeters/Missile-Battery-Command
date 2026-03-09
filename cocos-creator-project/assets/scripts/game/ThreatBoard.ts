import { TrackClassification, GameConstants, getTrackIdString, getClassificationString } from './GameTypes';
import { TrackManager } from './TrackManager';

// ============================================================================
// Threat Assessment Entry — one row on the threat board (Scope 2)
// ============================================================================

export interface ThreatEntry {
    trackId: number;
    trackIdString: string;
    classification: string;
    range: number;
    azimuth: number;
    altitude: number;
    speed: number;
    heading: number;
    inbound: boolean;
    closingRate: number;
    threatScore: number;
    timeToTerritory: number;
}

// ============================================================================
// ThreatBoard — the second scope heads-up display
// ============================================================================

export class ThreatBoard {
    static readonly MAX_DISPLAYED_THREATS = 5;

    private topThreats: ThreatEntry[] = [];

    update(trackManager: TrackManager): void {
        const allTracks = trackManager.getAllTracks();
        const candidates: ThreatEntry[] = [];

        for (const track of allTracks) {
            if (track.classification === TrackClassification.FRIENDLY) continue;
            if (!track.isAlive) continue;

            const inbound = ThreatBoard.isInbound(track.azimuth, track.heading);
            const closingRate = ThreatBoard.calculateClosingRate(
                track.range, track.azimuth, track.speed, track.heading);
            const threatScore = ThreatBoard.calculateThreatScore(track);
            const timeToTerritory = ThreatBoard.estimateTimeToTerritory(track.range, closingRate);

            candidates.push({
                trackId: track.trackId,
                trackIdString: getTrackIdString(track.trackId),
                classification: getClassificationString(track.classification),
                range: track.range,
                azimuth: track.azimuth,
                altitude: track.altitude,
                speed: track.speed,
                heading: track.heading,
                inbound,
                closingRate,
                threatScore,
                timeToTerritory,
            });
        }

        candidates.sort((a, b) => b.threatScore - a.threatScore);
        this.topThreats = candidates.slice(0, ThreatBoard.MAX_DISPLAYED_THREATS);
    }

    getTopThreats(): ThreatEntry[] { return this.topThreats; }
    getThreatCount(): number { return this.topThreats.length; }

    getThreat(rank: number): ThreatEntry | null {
        return (rank >= 0 && rank < this.topThreats.length) ? this.topThreats[rank] : null;
    }

    isOnBoard(trackId: number): boolean {
        return this.topThreats.some((t) => t.trackId === trackId);
    }

    getThreatByTrackId(trackId: number): ThreatEntry | null {
        return this.topThreats.find((t) => t.trackId === trackId) || null;
    }

    formatBoard(): string {
        let board = '=== THREAT BOARD (SCOPE 2) ===\n';
        board += `  TOP ${ThreatBoard.MAX_DISPLAYED_THREATS} THREATS\n`;
        board += '------------------------------\n';

        if (this.topThreats.length === 0) {
            board += '  NO THREATS DETECTED\n';
            return board;
        }

        for (let i = 0; i < this.topThreats.length; i++) {
            const t = this.topThreats[i];
            const eta = t.timeToTerritory < 9000 ? `${Math.floor(t.timeToTerritory)}s` : 'N/A';
            board += `#${i + 1} ${t.trackIdString} ${t.classification} ` +
                     `RNG:${Math.floor(t.range)}km AZ:${String(Math.floor(t.azimuth)).padStart(3, '0')} ` +
                     `SPD:${Math.floor(t.speed)}kts ${t.inbound ? 'INBOUND' : 'OUTBND '} ETA:${eta}\n`;
        }

        return board;
    }

    private static calculateThreatScore(track: { range: number; speed: number; azimuth: number; heading: number; classification: TrackClassification; altitude: number }): number {
        let score = 0;

        const proximityScore = ((GameConstants.RADAR_MAX_RANGE_KM - track.range) /
            GameConstants.RADAR_MAX_RANGE_KM) * 100.0;
        score += proximityScore * 3.0;

        const speedScore = (track.speed / 1000.0) * 50.0;
        score += speedScore;

        if (ThreatBoard.isInbound(track.azimuth, track.heading)) {
            score += 150.0;
        }

        if (track.classification === TrackClassification.HOSTILE) {
            score += 100.0;
        } else if (track.classification === TrackClassification.UNKNOWN) {
            score += 50.0;
        }

        if (track.altitude < 5000) {
            score += 30.0;
        }

        if (track.range <= GameConstants.TERRITORY_RADIUS_KM) {
            score += 200.0;
        }

        return score;
    }

    static isInbound(azimuth: number, heading: number): boolean {
        const inboundHeading = (azimuth + 180.0) % 360.0;
        let diff = ((Math.abs(heading - inboundHeading) + 360.0) % 360.0);
        if (diff > 180.0) diff = 360.0 - diff;
        return diff <= 60.0;
    }

    private static calculateClosingRate(range: number, azimuth: number, speed: number, heading: number): number {
        const speedKmPerSec = (speed * GameConstants.NM_TO_KM) / 3600.0;
        const inboundHeading = (azimuth + 180.0) % 360.0;
        const diff = ((heading - inboundHeading) * Math.PI) / 180.0;
        return speedKmPerSec * Math.cos(diff);
    }

    private static estimateTimeToTerritory(range: number, closingRate: number): number {
        if (closingRate <= 0.001) return 9999.0;
        const distToTerritory = range - GameConstants.TERRITORY_RADIUS_KM;
        if (distToTerritory <= 0) return 0;
        return distToTerritory / closingRate;
    }
}
