import { AircraftType, IFFStatus, GameConstants } from './GameTypes';
import { Aircraft } from './Aircraft';

// ============================================================================
// IFF (Identification Friend or Foe) System
// Automatic interrogation of radar contacts using Mode 4
// ============================================================================

interface IFFInterrogation {
    trackId: number;
    timeRemaining: number;
    aircraft: Aircraft;
}

export class IFFSystem {
    private pendingInterrogations: IFFInterrogation[] = [];
    private errorRate: number = 0;

    interrogate(aircraft: Aircraft): void {
        if (!aircraft || aircraft.getTrackId() < 0) return;
        if (this.isInterrogating(aircraft.getTrackId())) return;
        if (aircraft.getIFFStatus() !== IFFStatus.PENDING) return;

        this.pendingInterrogations.push({
            trackId: aircraft.getTrackId(),
            timeRemaining: GameConstants.IFF_INTERROGATION_TIME,
            aircraft,
        });
    }

    update(dt: number): void {
        this.pendingInterrogations = this.pendingInterrogations.filter((interrog) => {
            interrog.timeRemaining -= dt;

            if (interrog.timeRemaining <= 0) {
                if (interrog.aircraft && interrog.aircraft.isAlive()) {
                    const result = this.determineResult(interrog.aircraft);
                    interrog.aircraft.setIFFStatus(result);
                }
                return false; // remove completed
            }
            return true; // keep pending
        });
    }

    isInterrogating(trackId: number): boolean {
        return this.pendingInterrogations.some((i) => i.trackId === trackId);
    }

    setErrorRate(rate: number): void {
        this.errorRate = rate;
    }

    private determineResult(aircraft: Aircraft): IFFStatus {
        const actuallyFriendly = aircraft.isFriendly();

        if (Math.random() < this.errorRate) {
            const errorType = Math.random();
            if (errorType < 0.3) {
                return actuallyFriendly ? IFFStatus.HOSTILE : IFFStatus.FRIENDLY;
            }
            return IFFStatus.UNKNOWN;
        }

        if (aircraft.getType() === AircraftType.STEALTH_FIGHTER) {
            if (Math.random() < 0.3) {
                return IFFStatus.UNKNOWN;
            }
        }

        return actuallyFriendly ? IFFStatus.FRIENDLY : IFFStatus.HOSTILE;
    }
}
