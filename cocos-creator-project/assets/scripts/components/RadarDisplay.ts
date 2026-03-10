import { _decorator, Component, Graphics, Color, UITransform, Vec2, Label, Node } from 'cc';
import {
    AircraftType, TrackClassification, TrackData, BatteryData,
    GameConstants, getTrackIdString,
} from '../game/GameTypes';
import { TrackManager } from '../game/TrackManager';
import { FireControlSystem } from '../game/FireControlSystem';

const { ccclass, property } = _decorator;

// ============================================================================
// Radar PPI Display - renders the AN/TSQ-73 radar console view
// Cocos Creator component using Graphics for primitive drawing
// ============================================================================

@ccclass('RadarDisplay')
export class RadarDisplay extends Component {
    @property
    radius: number = 350;

    private graphics: Graphics | null = null;
    private sweepAngle: number = 0;
    private selectedTrackId: number = -1;
    private selectionPulseTimer: number = 0;

    private trackManager: TrackManager | null = null;
    private fireControl: FireControlSystem | null = null;

    private showTrackLabels: boolean = true;
    private showTrackTrails: boolean = true;
    private showNoise: boolean = true;
    private showWeather: boolean = true;

    // Noise points cached for performance
    private noisePoints: Array<{ x: number; y: number; brightness: number }> = [];
    private noiseRefreshTimer: number = 0;

    // Weather cloud cells — drifting patches of clutter on the PPI scope
    // Real radar sees moisture/precipitation as diffuse return that slowly
    // moves with wind, slightly washing out targets behind it.
    private cloudCells: Array<{
        x: number; y: number;         // center position in screen pixels
        radiusX: number;              // horizontal extent
        radiusY: number;              // vertical extent
        density: number;              // 0-1, how opaque/bright the return is
        driftX: number;               // pixels per second drift
        driftY: number;               // pixels per second drift
        // Sub-blobs that make the cloud look amorphous, not circular
        blobs: Array<{ ox: number; oy: number; r: number; brightness: number }>;
    }> = [];

    start(): void {
        this.graphics = this.getComponent(Graphics);
        if (!this.graphics) {
            this.graphics = this.addComponent(Graphics);
        }
        this.graphics!.lineWidth = 1;
        this.generateNoisePoints();
        this.generateCloudCells();
    }

    setTrackManager(mgr: TrackManager): void { this.trackManager = mgr; }
    setFireControlSystem(fcs: FireControlSystem): void { this.fireControl = fcs; }
    setSelectedTrack(trackId: number): void { this.selectedTrackId = trackId; }
    getSelectedTrack(): number { return this.selectedTrackId; }
    getSweepAngle(): number { return this.sweepAngle; }
    getRadius(): number { return this.radius; }
    setShowTrackLabels(show: boolean): void { this.showTrackLabels = show; }
    setShowTrackTrails(show: boolean): void { this.showTrackTrails = show; }
    setShowNoise(show: boolean): void { this.showNoise = show; }
    setShowWeather(show: boolean): void { this.showWeather = show; }

    update(dt: number): void {
        this.sweepAngle += GameConstants.RADAR_SWEEP_RATE_DPS * dt;
        if (this.sweepAngle >= 360) this.sweepAngle -= 360;

        this.selectionPulseTimer += dt;

        // Refresh noise periodically
        this.noiseRefreshTimer += dt;
        if (this.noiseRefreshTimer > 2.0) {
            this.noiseRefreshTimer = 0;
            this.generateNoisePoints();
        }

        // Drift cloud cells across the scope
        this.updateCloudCells(dt);

        this.drawAll();
    }

    private drawAll(): void {
        if (!this.graphics) return;
        this.graphics.clear();

        this.drawBackground();
        this.drawRangeRings();
        this.drawAzimuthLines();
        this.drawCenterCrosshair();
        this.drawTerritoryZone();
        if (this.showNoise) this.drawRadarNoise();
        if (this.showWeather) this.drawWeatherClutter();
        this.drawSweepBeam();
        if (this.fireControl) this.drawBatteryPositions();
        if (this.trackManager) {
            this.drawBlips();
            if (this.selectedTrackId >= 0) this.drawSelectedTrackHighlight();
        }
    }

    private drawBackground(): void {
        const g = this.graphics!;
        // Dark scope background
        g.fillColor = new Color(5, 15, 5, 255);
        g.circle(0, 0, this.radius);
        g.fill();

        // Scope border
        g.strokeColor = new Color(0, 160, 0, 255);
        g.lineWidth = 3;
        g.circle(0, 0, this.radius);
        g.stroke();
    }

    private drawRangeRings(): void {
        const g = this.graphics!;
        g.strokeColor = new Color(0, 100, 0, 255);
        g.lineWidth = 1;

        for (let i = 1; i <= GameConstants.RADAR_RANGE_RINGS; i++) {
            const ringRadius = (this.radius / GameConstants.RADAR_RANGE_RINGS) * i;
            g.circle(0, 0, ringRadius);
            g.stroke();
        }
    }

    private drawAzimuthLines(): void {
        const g = this.graphics!;
        g.strokeColor = new Color(0, 70, 0, 200);
        g.lineWidth = 1;

        for (let angle = 0; angle < 360; angle += 30) {
            const rad = (angle * Math.PI) / 180;
            const endX = this.radius * Math.sin(rad);
            const endY = this.radius * Math.cos(rad);
            g.moveTo(0, 0);
            g.lineTo(endX, endY);
            g.stroke();
        }
    }

    private drawCenterCrosshair(): void {
        const g = this.graphics!;
        g.strokeColor = new Color(0, 180, 0, 255);
        g.lineWidth = 2;
        const size = 10;
        g.moveTo(-size, 0);
        g.lineTo(size, 0);
        g.moveTo(0, -size);
        g.lineTo(0, size);
        g.stroke();
    }

    private drawTerritoryZone(): void {
        const g = this.graphics!;
        const territoryPixels = this.kmToPixels(GameConstants.TERRITORY_RADIUS_KM);

        // Dashed circle (draw segments)
        g.strokeColor = new Color(255, 140, 0, 180);
        g.lineWidth = 1;
        const segments = 36;
        for (let i = 0; i < segments; i += 2) {
            const startAngle = (i / segments) * Math.PI * 2;
            const endAngle = ((i + 1) / segments) * Math.PI * 2;
            g.moveTo(
                territoryPixels * Math.cos(startAngle),
                territoryPixels * Math.sin(startAngle),
            );
            g.arc(0, 0, territoryPixels, startAngle, endAngle, false);
            g.stroke();
        }
    }

    private drawSweepBeam(): void {
        const g = this.graphics!;
        const sweepRad = (this.sweepAngle * Math.PI) / 180;

        // Phosphor trail (~30 degree fade)
        const trailDegrees = 30;
        const trailSegments = 20;
        for (let i = 0; i < trailSegments; i++) {
            const frac = i / trailSegments;
            const angleDeg = this.sweepAngle - trailDegrees * frac;
            const angleRad = (angleDeg * Math.PI) / 180;
            const alpha = Math.floor(180 * (1 - frac));

            g.strokeColor = new Color(0, 255, 0, alpha);
            g.lineWidth = 2;
            g.moveTo(0, 0);
            g.lineTo(
                this.radius * Math.sin(angleRad),
                this.radius * Math.cos(angleRad),
            );
            g.stroke();
        }

        // Main sweep line
        g.strokeColor = new Color(0, 255, 0, 255);
        g.lineWidth = 2.5;
        g.moveTo(0, 0);
        g.lineTo(
            this.radius * Math.sin(sweepRad),
            this.radius * Math.cos(sweepRad),
        );
        g.stroke();
    }

    private drawRadarNoise(): void {
        const g = this.graphics!;
        for (const pt of this.noisePoints) {
            const dist = Math.sqrt(pt.x * pt.x + pt.y * pt.y);
            if (dist > this.radius) continue;

            g.fillColor = new Color(0, 120, 0, Math.floor(pt.brightness * 80));
            g.circle(pt.x, pt.y, 1);
            g.fill();
        }
    }

    private drawBlips(): void {
        if (!this.trackManager) return;
        const g = this.graphics!;
        const tracks = this.trackManager.getAllTracks();

        for (const track of tracks) {
            if (!track.isAlive) continue;

            const pos = this.polarToScreen(track.range, track.azimuth);
            let brightness = this.calculatePhosphorBrightness(track.azimuth);
            let size = this.getBlipSize(track.aircraftType);

            // Weather distortion — blips behind cloud cover get slight jitter
            // and reduced contrast, simulating moisture clutter washing out return
            const weatherFactor = this.getWeatherDistortion(pos.x, pos.y);
            let drawX = pos.x;
            let drawY = pos.y;
            if (weatherFactor > 0) {
                // Subtle position jitter (1-3 pixels max, proportional to density)
                const jitterScale = 3.0 * weatherFactor;
                drawX += (Math.random() - 0.5) * jitterScale;
                drawY += (Math.random() - 0.5) * jitterScale;
                // Wash out the blip slightly — reduce contrast against the
                // brighter weather background, but never fully obscure it
                brightness *= (1.0 - weatherFactor * 0.25);
                // Blip appears slightly bloated in weather clutter
                size *= (1.0 + weatherFactor * 0.15);
            }

            const color = this.getBlipColor(track.classification, brightness);

            // Draw blip
            g.fillColor = color;
            g.circle(drawX, drawY, size);
            g.fill();

            // Glow effect
            const glowAlpha = Math.floor(color.a * 0.3);
            g.fillColor = new Color(color.r, color.g, color.b, glowAlpha);
            g.circle(drawX, drawY, size * 2);
            g.fill();
        }
    }

    private drawSelectedTrackHighlight(): void {
        if (!this.trackManager || this.selectedTrackId < 0) return;

        const track = this.trackManager.getTrack(this.selectedTrackId);
        if (!track) return;

        const pos = this.polarToScreen(track.range, track.azimuth);
        const g = this.graphics!;

        // Pulsing selection bracket
        const pulse = 0.7 + 0.3 * Math.sin(this.selectionPulseTimer * 4);
        const bracketSize = 12 * pulse;

        g.strokeColor = new Color(255, 255, 0, 200);
        g.lineWidth = 2;

        // Top-left corner
        g.moveTo(pos.x - bracketSize, pos.y + bracketSize * 0.5);
        g.lineTo(pos.x - bracketSize, pos.y + bracketSize);
        g.lineTo(pos.x - bracketSize * 0.5, pos.y + bracketSize);

        // Top-right corner
        g.moveTo(pos.x + bracketSize * 0.5, pos.y + bracketSize);
        g.lineTo(pos.x + bracketSize, pos.y + bracketSize);
        g.lineTo(pos.x + bracketSize, pos.y + bracketSize * 0.5);

        // Bottom-right corner
        g.moveTo(pos.x + bracketSize, pos.y - bracketSize * 0.5);
        g.lineTo(pos.x + bracketSize, pos.y - bracketSize);
        g.lineTo(pos.x + bracketSize * 0.5, pos.y - bracketSize);

        // Bottom-left corner
        g.moveTo(pos.x - bracketSize * 0.5, pos.y - bracketSize);
        g.lineTo(pos.x - bracketSize, pos.y - bracketSize);
        g.lineTo(pos.x - bracketSize, pos.y - bracketSize * 0.5);

        g.stroke();
    }

    private drawBatteryPositions(): void {
        if (!this.fireControl) return;
        const g = this.graphics!;
        const batteries = this.fireControl.getAllBatteryData();

        for (const bat of batteries) {
            const pos = this.polarToScreen(bat.position.range, bat.position.azimuth);
            const size = 6;

            g.strokeColor = new Color(0, 200, 200, 180);
            g.lineWidth = 1.5;

            // Patriot = square, Hawk = triangle, Javelin = diamond
            switch (bat.type) {
                case 0: // PATRIOT
                    g.moveTo(pos.x - size, pos.y - size);
                    g.lineTo(pos.x + size, pos.y - size);
                    g.lineTo(pos.x + size, pos.y + size);
                    g.lineTo(pos.x - size, pos.y + size);
                    g.closePath();
                    g.stroke();
                    break;
                case 1: // HAWK
                    g.moveTo(pos.x, pos.y + size);
                    g.lineTo(pos.x + size, pos.y - size);
                    g.lineTo(pos.x - size, pos.y - size);
                    g.closePath();
                    g.stroke();
                    break;
                case 2: // JAVELIN
                    g.moveTo(pos.x, pos.y + size);
                    g.lineTo(pos.x + size, pos.y);
                    g.lineTo(pos.x, pos.y - size);
                    g.lineTo(pos.x - size, pos.y);
                    g.closePath();
                    g.stroke();
                    break;
            }
        }
    }

    // Convert polar coordinates (range km, azimuth deg) to screen position
    polarToScreen(range: number, azimuth: number): { x: number; y: number } {
        const pixels = this.kmToPixels(range);
        const rad = (azimuth * Math.PI) / 180;
        return {
            x: pixels * Math.sin(rad),
            y: pixels * Math.cos(rad),
        };
    }

    kmToPixels(km: number): number {
        return (km / GameConstants.RADAR_MAX_RANGE_KM) * this.radius;
    }

    // Find nearest track to a screen-space point
    findNearestTrack(localX: number, localY: number, maxDist: number = 20): number {
        if (!this.trackManager) return -1;

        const tracks = this.trackManager.getAllTracks();
        let nearestId = -1;
        let nearestDist = maxDist;

        for (const track of tracks) {
            if (!track.isAlive) continue;
            const pos = this.polarToScreen(track.range, track.azimuth);
            const dx = localX - pos.x;
            const dy = localY - pos.y;
            const dist = Math.sqrt(dx * dx + dy * dy);

            if (dist < nearestDist) {
                nearestDist = dist;
                nearestId = track.trackId;
            }
        }

        return nearestId;
    }

    // Phosphor brightness based on sweep position
    private calculatePhosphorBrightness(blipAzimuth: number): number {
        let diff = this.sweepAngle - blipAzimuth;
        if (diff < 0) diff += 360;
        if (diff > 360) diff -= 360;

        // Brightness decays as the sweep moves past
        const fadeAngle = GameConstants.RADAR_SWEEP_RATE_DPS * GameConstants.BLIP_FADE_TIME;
        if (diff < fadeAngle) {
            return 1.0 - diff / fadeAngle;
        }
        return 0.05; // minimum residual glow
    }

    private getBlipSize(type: AircraftType): number {
        switch (type) {
            case AircraftType.STRATEGIC_BOMBER:  return 5;
            case AircraftType.TACTICAL_BOMBER:   return 4;
            case AircraftType.FIGHTER_ATTACK:    return 3.5;
            case AircraftType.CIVILIAN_AIRLINER: return 4.5;
            case AircraftType.FRIENDLY_MILITARY: return 3.5;
            case AircraftType.STEALTH_FIGHTER:   return 2;
            case AircraftType.ATTACK_DRONE:      return 2.5;
            case AircraftType.RECON_DRONE:       return 2;
        }
        return 3;
    }

    private getBlipColor(classification: TrackClassification, brightness: number): Color {
        const alpha = Math.floor(brightness * 255);
        switch (classification) {
            case TrackClassification.HOSTILE:
                return new Color(255, 50, 50, alpha);
            case TrackClassification.FRIENDLY:
                return new Color(50, 100, 255, alpha);
            case TrackClassification.UNKNOWN:
                return new Color(255, 200, 0, alpha);
            case TrackClassification.PENDING:
                return new Color(150, 150, 150, alpha);
        }
    }

    private generateNoisePoints(): void {
        this.noisePoints = [];
        const count = 120;
        for (let i = 0; i < count; i++) {
            // Quadratic distribution — more noise near center
            const r = Math.sqrt(Math.random()) * this.radius;
            const angle = Math.random() * Math.PI * 2;
            this.noisePoints.push({
                x: r * Math.cos(angle),
                y: r * Math.sin(angle),
                brightness: Math.random(),
            });
        }
    }

    // ========================================================================
    // Weather cloud layer — local cloud cover drifting across the PPI scope
    //
    // On a real radar, moisture and precipitation appear as diffuse areas of
    // return that slowly move with the prevailing wind. They brighten slightly
    // when the sweep beam passes through them and slightly wash out any
    // aircraft returns behind them.
    // ========================================================================

    private generateCloudCells(): void {
        this.cloudCells = [];
        // 3-5 cloud patches at random positions within the scope
        const count = 3 + Math.floor(Math.random() * 3);
        for (let i = 0; i < count; i++) {
            const angle = Math.random() * Math.PI * 2;
            const dist = (0.2 + Math.random() * 0.6) * this.radius;
            const cx = dist * Math.cos(angle);
            const cy = dist * Math.sin(angle);

            // Elliptical extent — clouds are wider than tall (wind-stretched)
            const radiusX = 30 + Math.random() * 60;
            const radiusY = 20 + Math.random() * 40;
            const density = 0.3 + Math.random() * 0.5;

            // Slow drift — prevailing wind roughly from one direction
            // with some variation per cell
            const windAngle = Math.PI * 0.7 + (Math.random() - 0.5) * 0.4;
            const windSpeed = 2 + Math.random() * 4; // pixels per second
            const driftX = Math.cos(windAngle) * windSpeed;
            const driftY = Math.sin(windAngle) * windSpeed;

            // Generate 4-8 sub-blobs to make the shape amorphous
            const blobCount = 4 + Math.floor(Math.random() * 5);
            const blobs: Array<{ ox: number; oy: number; r: number; brightness: number }> = [];
            for (let b = 0; b < blobCount; b++) {
                blobs.push({
                    ox: (Math.random() - 0.5) * radiusX * 1.2,
                    oy: (Math.random() - 0.5) * radiusY * 1.2,
                    r: 10 + Math.random() * 25,
                    brightness: 0.4 + Math.random() * 0.6,
                });
            }

            this.cloudCells.push({
                x: cx, y: cy, radiusX, radiusY, density,
                driftX, driftY, blobs,
            });
        }
    }

    private updateCloudCells(dt: number): void {
        for (const cell of this.cloudCells) {
            cell.x += cell.driftX * dt;
            cell.y += cell.driftY * dt;

            // If a cloud drifts off the scope, wrap it around from the
            // opposite edge (weather continuously moves through the area)
            const dist = Math.sqrt(cell.x * cell.x + cell.y * cell.y);
            if (dist > this.radius + cell.radiusX) {
                // Re-enter from opposite side
                const angle = Math.atan2(cell.y, cell.x) + Math.PI;
                cell.x = Math.cos(angle) * (this.radius * 0.8);
                cell.y = Math.sin(angle) * (this.radius * 0.8);
            }
        }
    }

    private drawWeatherClutter(): void {
        const g = this.graphics!;

        for (const cell of this.cloudCells) {
            // Each cloud is rendered as overlapping soft circles (sub-blobs)
            // with very low alpha — subtle diffuse green return
            for (const blob of cell.blobs) {
                const bx = cell.x + blob.ox;
                const by = cell.y + blob.oy;

                // Clip to scope radius
                const dist = Math.sqrt(bx * bx + by * by);
                if (dist > this.radius) continue;

                // Brightness modulated by sweep proximity — weather return
                // is most visible right after the sweep passes through it
                const blobAzimuth = ((Math.atan2(bx, by) * 180) / Math.PI + 360) % 360;
                const sweepProximity = this.calculatePhosphorBrightness(blobAzimuth);
                const alpha = Math.floor(
                    cell.density * blob.brightness * sweepProximity * 45
                );
                if (alpha < 2) continue;

                g.fillColor = new Color(0, 100, 0, alpha);
                g.circle(bx, by, blob.r);
                g.fill();

                // Outer haze — even fainter surrounding glow
                const hazeAlpha = Math.floor(alpha * 0.3);
                if (hazeAlpha > 1) {
                    g.fillColor = new Color(0, 80, 0, hazeAlpha);
                    g.circle(bx, by, blob.r * 1.5);
                    g.fill();
                }
            }
        }
    }

    // Returns 0-1 indicating how much weather clutter affects a given
    // screen position. Used by drawBlips to distort tracks behind clouds.
    private getWeatherDistortion(sx: number, sy: number): number {
        if (!this.showWeather) return 0;

        let maxDistortion = 0;
        for (const cell of this.cloudCells) {
            // Elliptical distance from cloud center
            const dx = (sx - cell.x) / cell.radiusX;
            const dy = (sy - cell.y) / cell.radiusY;
            const ellipseDist = dx * dx + dy * dy;

            if (ellipseDist < 1.0) {
                // Inside the cloud envelope — distortion is strongest at center
                const falloff = 1.0 - ellipseDist;
                const distortion = falloff * cell.density;
                if (distortion > maxDistortion) {
                    maxDistortion = distortion;
                }
            }
        }
        return maxDistortion;
    }
}
