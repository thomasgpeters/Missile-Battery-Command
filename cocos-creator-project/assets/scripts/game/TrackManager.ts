import { TrackData, TrackClassification, GameConstants } from './GameTypes';
import { Aircraft } from './Aircraft';
import { IFFSystem } from './IFFSystem';

// ============================================================================
// Track Manager - assigns and manages track IDs for all radar contacts
// ============================================================================

interface TrackEntry {
    trackId: number;
    aircraft: Aircraft;
    cachedData: TrackData;
}

export class TrackManager {
    private tracks: TrackEntry[] = [];
    private nextTrackId: number = 1;
    private iffSystem: IFFSystem = new IFFSystem();

    addTrack(aircraft: Aircraft): number {
        if (!aircraft) return -1;

        const id = this.nextTrackId++;
        aircraft.setTrackId(id);

        this.tracks.push({
            trackId: id,
            aircraft,
            cachedData: aircraft.getTrackData(),
        });

        this.iffSystem.interrogate(aircraft);
        return id;
    }

    removeTrack(trackId: number): void {
        this.tracks = this.tracks.filter((e) => e.trackId !== trackId);
    }

    update(dt: number): void {
        this.iffSystem.update(dt);

        this.tracks = this.tracks.filter((entry) => {
            if (!entry.aircraft || !entry.aircraft.isAlive() ||
                entry.aircraft.getRange() > GameConstants.RADAR_MAX_RANGE_KM) {
                return false;
            }
            entry.cachedData = entry.aircraft.getTrackData();
            return true;
        });
    }

    getAllTracks(): TrackData[] {
        return this.tracks.map((e) => e.cachedData);
    }

    getTrack(trackId: number): TrackData | null {
        const entry = this.tracks.find((e) => e.trackId === trackId);
        return entry ? entry.cachedData : null;
    }

    getAircraftByTrackId(trackId: number): Aircraft | null {
        const entry = this.tracks.find((e) => e.trackId === trackId);
        return entry ? entry.aircraft : null;
    }

    getHostileTrackIds(): number[] {
        return this.tracks
            .filter((e) => e.cachedData.classification === TrackClassification.HOSTILE)
            .map((e) => e.trackId);
    }

    getActiveTrackCount(): number { return this.tracks.length; }

    getHostileCount(): number {
        return this.tracks.filter(
            (e) => e.cachedData.classification === TrackClassification.HOSTILE,
        ).length;
    }

    getFriendlyCount(): number {
        return this.tracks.filter(
            (e) => e.cachedData.classification === TrackClassification.FRIENDLY,
        ).length;
    }

    getIFFSystem(): IFFSystem { return this.iffSystem; }

    reset(): void {
        this.tracks = [];
        this.nextTrackId = 1;
    }
}
