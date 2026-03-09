# Product Backlog

## Missile Battery Command — Prioritized Feature List

Items are ordered by priority within each tier. Higher tiers must be completed before lower tiers for a playable, graphically realistic game.

---

## Tier 1: Critical Path — Graphical Radar Console (COMPLETE)

All Tier 1 items have been implemented. The AN/TSQ-73 console renders a full PPI radar scope with cocos2d-x, sweep beam, blips, track selection, HUD panels, and keyboard fire control.

| # | Feature | Description | Status |
|---|---------|-------------|--------|
| 1.1 | **cocos2d-x Integration & Build** | cocos2d-x linked, compiling, rendering at 1280x720. Stub mode fallback. | DONE |
| 1.2 | **PPI Radar Scope Rendering** | Circular scope with range rings (NM), azimuth lines, center crosshair. AN/TPS-43E 250 NM range. | DONE |
| 1.3 | **Rotating Sweep Beam** | 6 RPM sweep with 30° phosphor trail, non-linear decay, 20-segment gradient. | DONE |
| 1.4 | **Radar Blip Rendering** | RCS-sized blips with phosphor brightness tied to sweep timing. Glow effect. | DONE |
| 1.5 | **Blip Color Coding by IFF** | Red (hostile), blue (friendly), yellow (unknown), gray (pending). | DONE |
| 1.6 | **Track Labels on Scope** | Track ID (TK-001) displayed adjacent to each blip. | DONE |
| 1.7 | **Track Selection via Click/Touch** | Click blip to select, highlight bracket + ring, click empty to deselect. | DONE |
| 1.8 | **HUD Track Info Panel** | Full tactical data: Track ID, IFF, altitude, azimuth, range (NM+km), speed, heading. | DONE |
| 1.9 | **HUD Battery Status Panel** | All batteries with designation, status, missile count. Available battery highlighting. | DONE |
| 1.10 | **Keyboard Fire Control** | Keys 1-9 assign batteries, F fires, A aborts. Visual feedback in message log. | DONE |

## Tier 2: Core Gameplay Loop (Must Have)

| # | Feature | Description | Depends On | Status |
|---|---------|-------------|------------|--------|
| 2.1 | **Missile Launch Visualization** | Animate missile from battery to target. Dashed cyan trail on scope. | 1.10 | DONE |
| 2.2 | **Engagement Result Visualization** | Flash/burst at target. "SPLASH" for kills, "MISS" for misses. | 2.1 | Planned |
| 2.3 | **Score Display and Tracking** | Persistent score counter. Score popup animations at event locations. | 1.8 | DONE (counter) |
| 2.4 | **Message Log / Engagement Feed** | Scrolling text feed: contacts, IFF, engagements, kills, warnings. | 1.8 | DONE |
| 2.5 | **Game Over Condition & Screen** | Game over when score < -2000 or too many penetrations. Restart option. | 2.3 | Planned |
| 2.6 | **Level Progression** | Auto-advance based on score/time. Level indicator. Transition announcement. | 2.5 | Planned |
| 2.7 | **Hostile Penetration Warning** | Alert when hostile reaches defense zone center. Scope flash, alarm text. | 1.4 | Planned |
| 2.8 | **Friendly Fire Warning** | Full-screen red flash, "FRIENDLY FIRE" warning. Heavy penalty. | 2.2 | Planned |

## Tier 3: Visual Polish & Realism (Should Have)

| # | Feature | Description | Depends On | Status |
|---|---------|-------------|------------|--------|
| 3.1 | **Radar Noise / Clutter** | Random green dots simulating ground clutter. Quadratic radial distribution. | 1.3 | DONE |
| 3.2 | **Track History Trail** | 8-point trail, 1.5s intervals, 15s max age, fading opacity. | 1.4 | DONE |
| 3.3 | **Battery Position Icons** | P1-P3, H1-H3, J1-J3 icons with status-coded colors on scope. | 1.2 | DONE |
| 3.4 | **Phosphor Burn-in Effect** | Static elements glow brighter. Speed-dependent trail length. | 1.3 | Planned |
| 3.5 | **CRT Screen Effect Shader** | Barrel distortion, scanlines, vignette, green bleed, chromatic aberration. | 1.2 | Planned |
| 3.6 | **Explosion Particles** | Particle effects for missile detonation. Debris particles. | 2.2 | Planned |
| 3.7 | **Console Bezel / Frame** | AN/TSQ-73 console housing border around radar scope. | 1.2 | Planned |
| 3.8 | **Scope Brightness Pulsing** | Subtle CRT power fluctuation simulation. | 1.3 | Planned |

## Tier 4: Audio (Should Have)

| # | Feature | Description | Depends On | Status |
|---|---------|-------------|------------|--------|
| 4.1 | **Radar Sweep Tone** | Ticking/pinging synced with sweep rotation. | 1.3 | Planned |
| 4.2 | **New Contact Alert** | Audio alert on new radar contact. | 1.4 | Planned |
| 4.3 | **IFF Response Tone** | Hostile (buzz), friendly (beep), unknown (question tone). | 1.5 | Planned |
| 4.4 | **Missile Launch Sound** | Rocket ignition on fire command. | 2.1 | Planned |
| 4.5 | **Explosion / Impact Sound** | Explosion for hits, whoosh for misses. | 2.2 | Planned |
| 4.6 | **Warning Klaxon** | Alarm for penetration, friendly fire, low missiles. | 2.7, 2.8 | Planned |
| 4.7 | **Ambient Console Hum** | Background electronics and cooling fan hum. | 1.1 | Planned |

## Tier 5: Advanced Gameplay (Nice to Have)

| # | Feature | Description | Depends On | Status |
|---|---------|-------------|------------|--------|
| 5.1 | **Electronic Countermeasures (ECM)** | Hostile radar jamming, ghost returns, blip flickering. | 1.4 | Planned |
| 5.2 | **SATCOM Intelligence Updates** | Periodic intel text messages providing advance raid warning. | 2.4 | Planned |
| 5.3 | **Battery Relocation** | Batteries go offline during move, strategic repositioning. | 1.9 | DONE |
| 5.4 | **Missile Resupply** | Resupply convoy, player must protect. Replenishes batteries. | 2.6 | Planned |
| 5.5 | **Multi-Target Engagement** | Multiple batteries on same target. Coordinated salvo. | 1.10 | Planned |
| 5.6 | **Aircraft Formation Raids** | V-formation bombers with fighter escorts. Raid designation. | Aircraft | Planned |
| 5.7 | **Terrain Masking** | Low-altitude aircraft hide behind terrain. | 1.4 | Planned |
| 5.8 | **Night/Day Cycle Visual** | Scope aesthetics shift with time of day. | 3.5 | Planned |
| 5.9 | **Threat Board (Scope 2)** | Top 5 threats ranked by composite score. | Track Mgr | DONE |
| 5.10 | **Mobile Battalion HQ** | 90s relocation cycle, phased radar/comms recovery. | — | DONE |
| 5.11 | **Airspace Management** | No-fly zones, air corridors, restricted areas. | — | DONE |
| 5.12 | **Equipment Data Cards** | Equipment, troop strength, and intel cards. | — | DONE |

## Tier 6: Metagame & UI (Nice to Have)

| # | Feature | Description | Depends On | Status |
|---|---------|-------------|------------|--------|
| 6.1 | **Main Menu** | Title screen with military aesthetic. Start, Settings, Scores, Quit. | 1.1 | Planned |
| 6.2 | **Settings Screen** | Difficulty, audio, key bindings, display options. | 6.1 | Planned |
| 6.3 | **High Score Table** | Persistent local top 10. Name entry. | 2.5 | Planned |
| 6.4 | **Pause Menu** | Resume, restart, settings, quit to menu. | 6.1 | Planned |
| 6.5 | **Mission Briefing** | Pre-game scenario, threat types, assets, objectives. | 6.1 | Planned |
| 6.6 | **After Action Report** | Post-game summary: kills, accuracy %, grade A-F. | 2.5 | Planned |
| 6.7 | **Tutorial / Training Mode** | Guided first-play with highlighted UI, slow pace. | All Tier 2 | Planned |
| 6.8 | **Campaign Mode** | 10 escalating missions with narrative. Unlockable. | 6.5, 6.6 | Planned |

---

## Priority Legend

| Tier | Priority | Gate | Status |
|------|----------|------|--------|
| Tier 1 | **CRITICAL** | Required for first playable build | COMPLETE |
| Tier 2 | **CRITICAL** | Required for complete gameplay loop | IN PROGRESS |
| Tier 3 | **HIGH** | Required for graphical realism target | PARTIAL (3 of 8 done) |
| Tier 4 | **HIGH** | Required for immersive experience | Not started |
| Tier 5 | **MEDIUM** | Deepens tactical gameplay | PARTIAL (5 of 12 done) |
| Tier 6 | **MEDIUM** | Completes the product | Not started |
