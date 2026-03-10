import { _decorator, Component, Graphics, Color, UITransform } from 'cc';

const { ccclass, property } = _decorator;

// ============================================================================
// ConsoleFrame - Draws the AN/TSQ-73 display console housing around the
// PPI radar scope. Based on the actual Display Console Assembly (items 11/13
// in the S-280 shelter arrangement).
//
// The console has:
//   - A metal bezel/housing with rubber scope hood
//   - Illuminated pushbutton panels flanking the scope (left and right)
//   - Knobs and switches below the scope
//   - The whole unit sits in a dark shelter interior
// ============================================================================

@ccclass('ConsoleFrame')
export class ConsoleFrame extends Component {
    @property
    scopeRadius: number = 280;

    @property
    panelWidth: number = 160;

    private graphics: Graphics | null = null;

    start(): void {
        this.graphics = this.getComponent(Graphics);
        if (!this.graphics) {
            this.graphics = this.addComponent(Graphics);
        }
        this.drawFrame();
    }

    drawFrame(): void {
        if (!this.graphics) return;
        this.graphics.clear();

        this.drawShelterBackground();
        this.drawConsoleHousing();
        this.drawScopeBezel();
        this.drawLeftButtonPanel();
        this.drawRightButtonPanel();
        this.drawBottomControlStrip();
        this.drawHousingLabels();
    }

    private drawShelterBackground(): void {
        const g = this.graphics!;
        // Dark shelter interior — the S-280 with blackout curtain (item 8)
        g.fillColor = new Color(15, 15, 18, 255);
        g.rect(-640, -480, 1280, 960);
        g.fill();
    }

    private drawConsoleHousing(): void {
        const g = this.graphics!;
        const r = this.scopeRadius;
        const pw = this.panelWidth;

        // Main console housing — olive drab metal panel
        const housingLeft = -r - pw - 30;
        const housingRight = r + pw + 30;
        const housingTop = r + 40;
        const housingBottom = -r - 80;
        const housingWidth = housingRight - housingLeft;
        const housingHeight = housingTop - housingBottom;

        // Console body — dark olive metal
        g.fillColor = new Color(45, 48, 40, 255);
        g.roundRect(housingLeft, housingBottom, housingWidth, housingHeight, 6);
        g.fill();

        // Subtle bevel highlight on top edge
        g.strokeColor = new Color(70, 75, 62, 255);
        g.lineWidth = 2;
        g.moveTo(housingLeft + 6, housingTop);
        g.lineTo(housingRight - 6, housingTop);
        g.stroke();

        // Shadow on bottom edge
        g.strokeColor = new Color(25, 28, 22, 255);
        g.lineWidth = 2;
        g.moveTo(housingLeft + 6, housingBottom);
        g.lineTo(housingRight - 6, housingBottom);
        g.stroke();

        // Console outline
        g.strokeColor = new Color(60, 65, 52, 255);
        g.lineWidth = 1;
        g.roundRect(housingLeft, housingBottom, housingWidth, housingHeight, 6);
        g.stroke();
    }

    private drawScopeBezel(): void {
        const g = this.graphics!;
        const r = this.scopeRadius;

        // Rubber scope hood — dark ring around the PPI
        // Outer ring (rubber hood edge)
        g.fillColor = new Color(20, 20, 20, 255);
        g.circle(0, 0, r + 18);
        g.fill();

        // Inner hood bevel
        g.fillColor = new Color(30, 30, 30, 255);
        g.circle(0, 0, r + 12);
        g.fill();

        // Metal bezel ring
        g.strokeColor = new Color(80, 85, 72, 255);
        g.lineWidth = 3;
        g.circle(0, 0, r + 18);
        g.stroke();

        // Inner bezel ring
        g.strokeColor = new Color(50, 55, 45, 255);
        g.lineWidth = 2;
        g.circle(0, 0, r + 6);
        g.stroke();

        // Scope mounting screws (4 corners)
        const screwDist = r + 15;
        const screwPositions = [
            { x: -screwDist * 0.707, y: screwDist * 0.707 },
            { x: screwDist * 0.707, y: screwDist * 0.707 },
            { x: screwDist * 0.707, y: -screwDist * 0.707 },
            { x: -screwDist * 0.707, y: -screwDist * 0.707 },
        ];
        for (const screw of screwPositions) {
            g.fillColor = new Color(90, 95, 82, 255);
            g.circle(screw.x, screw.y, 4);
            g.fill();
            g.strokeColor = new Color(50, 55, 45, 255);
            g.lineWidth = 1;
            g.circle(screw.x, screw.y, 4);
            g.stroke();
            // Phillips slot
            g.strokeColor = new Color(60, 65, 52, 255);
            g.lineWidth = 1;
            g.moveTo(screw.x - 2, screw.y);
            g.lineTo(screw.x + 2, screw.y);
            g.moveTo(screw.x, screw.y - 2);
            g.lineTo(screw.x, screw.y + 2);
            g.stroke();
        }
    }

    private drawLeftButtonPanel(): void {
        const g = this.graphics!;
        const r = this.scopeRadius;
        const pw = this.panelWidth;

        // Left panel background
        const panelX = -r - pw - 20;
        const panelY = -r + 20;
        const panelH = r * 2 - 40;

        g.fillColor = new Color(38, 42, 35, 255);
        g.roundRect(panelX, panelY, pw, panelH, 3);
        g.fill();
        g.strokeColor = new Color(55, 60, 48, 255);
        g.lineWidth = 1;
        g.roundRect(panelX, panelY, pw, panelH, 3);
        g.stroke();

        // Panel label area at top
        g.fillColor = new Color(30, 33, 28, 255);
        g.roundRect(panelX + 10, panelY + panelH - 28, pw - 20, 20, 2);
        g.fill();

        // Illuminated pushbuttons — 4 columns x 8 rows
        // These are the BATTERY ASSIGNMENT and TRACK MANAGEMENT buttons
        const btnCols = 4;
        const btnRows = 8;
        const btnW = 28;
        const btnH = 22;
        const btnGapX = 6;
        const btnGapY = 6;
        const startX = panelX + 14;
        const startY = panelY + panelH - 60;

        const leftBtnColors = [
            // Row labels: battery assignment buttons (amber/yellow lit)
            new Color(180, 150, 40, 255),  // Active/lit
            new Color(80, 70, 25, 255),    // Dim/standby
            new Color(60, 55, 20, 255),    // Off
        ];

        for (let row = 0; row < btnRows; row++) {
            for (let col = 0; col < btnCols; col++) {
                const bx = startX + col * (btnW + btnGapX);
                const by = startY - row * (btnH + btnGapY);

                // Button housing
                g.fillColor = new Color(25, 28, 22, 255);
                g.roundRect(bx, by, btnW, btnH, 2);
                g.fill();

                // Button face — some lit, some dim
                const isLit = (row < 3 && col < 3); // Top-left cluster lit
                const isDim = (row >= 3 && row < 6);
                let btnColor: Color;
                if (isLit) {
                    btnColor = leftBtnColors[0];
                } else if (isDim) {
                    btnColor = leftBtnColors[1];
                } else {
                    btnColor = leftBtnColors[2];
                }

                g.fillColor = btnColor;
                g.roundRect(bx + 2, by + 2, btnW - 4, btnH - 4, 1);
                g.fill();

                // Button border
                g.strokeColor = new Color(90, 95, 80, 255);
                g.lineWidth = 1;
                g.roundRect(bx, by, btnW, btnH, 2);
                g.stroke();
            }
        }
    }

    private drawRightButtonPanel(): void {
        const g = this.graphics!;
        const r = this.scopeRadius;
        const pw = this.panelWidth;

        // Right panel background
        const panelX = r + 20;
        const panelY = -r + 20;
        const panelH = r * 2 - 40;

        g.fillColor = new Color(38, 42, 35, 255);
        g.roundRect(panelX, panelY, pw, panelH, 3);
        g.fill();
        g.strokeColor = new Color(55, 60, 48, 255);
        g.lineWidth = 1;
        g.roundRect(panelX, panelY, pw, panelH, 3);
        g.stroke();

        // Panel label
        g.fillColor = new Color(30, 33, 28, 255);
        g.roundRect(panelX + 10, panelY + panelH - 28, pw - 20, 20, 2);
        g.fill();

        // IFF and ENGAGEMENT buttons — green/red indicators
        const btnCols = 4;
        const btnRows = 8;
        const btnW = 28;
        const btnH = 22;
        const btnGapX = 6;
        const btnGapY = 6;
        const startX = panelX + 14;
        const startY = panelY + panelH - 60;

        for (let row = 0; row < btnRows; row++) {
            for (let col = 0; col < btnCols; col++) {
                const bx = startX + col * (btnW + btnGapX);
                const by = startY - row * (btnH + btnGapY);

                // Button housing
                g.fillColor = new Color(25, 28, 22, 255);
                g.roundRect(bx, by, btnW, btnH, 2);
                g.fill();

                // Right panel buttons — mix of green (IFF) and red (weapons)
                let btnColor: Color;
                if (col < 2 && row < 3) {
                    // Green — IFF/track management
                    btnColor = new Color(40, 140, 40, 255);
                } else if (col >= 2 && row < 2) {
                    // Red — weapons/fire control
                    btnColor = new Color(160, 40, 40, 255);
                } else if (row >= 3 && row < 5) {
                    btnColor = new Color(60, 80, 60, 255); // Dim green
                } else {
                    btnColor = new Color(50, 50, 45, 255); // Off
                }

                g.fillColor = btnColor;
                g.roundRect(bx + 2, by + 2, btnW - 4, btnH - 4, 1);
                g.fill();

                g.strokeColor = new Color(90, 95, 80, 255);
                g.lineWidth = 1;
                g.roundRect(bx, by, btnW, btnH, 2);
                g.stroke();
            }
        }
    }

    private drawBottomControlStrip(): void {
        const g = this.graphics!;
        const r = this.scopeRadius;
        const pw = this.panelWidth;

        // Strip below the scope — knobs and switches
        const stripX = -r - pw - 20;
        const stripY = -r - 70;
        const stripW = (r + pw + 20) * 2;
        const stripH = 50;

        g.fillColor = new Color(40, 44, 36, 255);
        g.roundRect(stripX, stripY, stripW, stripH, 3);
        g.fill();
        g.strokeColor = new Color(55, 60, 48, 255);
        g.lineWidth = 1;
        g.roundRect(stripX, stripY, stripW, stripH, 3);
        g.stroke();

        // Rotary knobs — brightness, contrast, focus, range
        const knobY = stripY + stripH / 2;
        const knobPositions = [-200, -80, 0, 80, 200];
        for (const kx of knobPositions) {
            // Knob base
            g.fillColor = new Color(30, 33, 28, 255);
            g.circle(kx, knobY, 14);
            g.fill();

            // Knob cap
            g.fillColor = new Color(55, 60, 48, 255);
            g.circle(kx, knobY, 10);
            g.fill();

            // Knob indicator line
            g.strokeColor = new Color(180, 180, 170, 255);
            g.lineWidth = 2;
            g.moveTo(kx, knobY);
            g.lineTo(kx, knobY + 8);
            g.stroke();

            // Knob ring
            g.strokeColor = new Color(80, 85, 72, 255);
            g.lineWidth = 1;
            g.circle(kx, knobY, 14);
            g.stroke();
        }

        // Toggle switches between knobs
        const switchPositions = [-140, -40, 40, 140];
        for (const sx of switchPositions) {
            // Switch housing
            g.fillColor = new Color(30, 33, 28, 255);
            g.roundRect(sx - 6, knobY - 10, 12, 20, 2);
            g.fill();

            // Switch toggle (up position)
            g.fillColor = new Color(160, 160, 150, 255);
            g.roundRect(sx - 3, knobY - 2, 6, 12, 1);
            g.fill();
        }
    }

    private drawHousingLabels(): void {
        const g = this.graphics!;
        const r = this.scopeRadius;

        // Small indicator lights along the top of the housing
        const indicatorY = r + 28;
        const indicators = [
            { x: -100, color: new Color(0, 200, 0, 255), label: 'PWR' },
            { x: -50,  color: new Color(200, 200, 0, 255), label: 'RDR' },
            { x: 0,    color: new Color(0, 200, 0, 255), label: 'IFF' },
            { x: 50,   color: new Color(200, 0, 0, 180), label: 'WPN' },
            { x: 100,  color: new Color(0, 200, 0, 255), label: 'COM' },
        ];

        for (const ind of indicators) {
            // LED indicator
            g.fillColor = ind.color;
            g.circle(ind.x, indicatorY, 4);
            g.fill();

            // LED glow
            g.fillColor = new Color(ind.color.r, ind.color.g, ind.color.b, 40);
            g.circle(ind.x, indicatorY, 8);
            g.fill();
        }

        // Manufacturer plate (subtle)
        g.fillColor = new Color(50, 55, 45, 255);
        g.roundRect(-60, -r - 55, 120, 14, 2);
        g.fill();
        g.strokeColor = new Color(65, 70, 58, 255);
        g.lineWidth = 1;
        g.roundRect(-60, -r - 55, 120, 14, 2);
        g.stroke();
    }
}
