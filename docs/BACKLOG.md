# Product Backlog

## Missile Battery Command — Prioritized Feature List

Items are ordered by priority within each tier. Higher tiers must be completed before lower tiers for a playable, graphically realistic game.

---

## Tier 1: Critical Path — Graphical Radar Console (Must Have)

These items are required to go from console stub to a playable graphical game.

| # | Feature | Description | Depends On |
|---|---------|-------------|------------|
| 1.1 | **cocos2d-x Integration & Build** | Get cocos2d-x linked, compiling, and rendering a window with the correct resolution (1280x720). Verify on target platform. | — |
| 1.2 | **PPI Radar Scope Rendering** | Render the circular radar scope with dark background, green phosphor aesthetic. Range rings with labeled distances (20/40/60/80/100 km). Azimuth tick marks at 30-degree intervals with degree labels. Center crosshair. | 1.1 |
| 1.3 | **Rotating Sweep Beam** | Animated radar sweep beam rotating clockwise at 6 RPM (360 degrees every 10 seconds). Green gradient from bright at leading edge to transparent at trailing edge. Phosphor glow trail effect. | 1.2 |
| 1.4 | **Radar Blip Rendering** | Render aircraft contacts as blips on the PPI scope. Blips brighten when the sweep beam passes over them and fade (phosphor decay) between sweeps. Size varies by radar cross-section. | 1.3 |
| 1.5 | **Blip Color Coding by IFF** | Color-code blips: red (HOSTILE), blue (FRIENDLY), yellow (UNKNOWN), gray (PENDING). Smooth color transition when IFF status changes. | 1.4 |
| 1.6 | **Track Labels on Scope** | Display Track ID (TK-001) next to each blip. Small text, offset to avoid overlap with blip. Toggle-able. | 1.5 |
| 1.7 | **Track Selection via Click/Touch** | Click or tap a blip to select it. Selected track gets a highlight ring/bracket. Selection updates the HUD track info panel. | 1.6 |
| 1.8 | **HUD Track Info Panel** | Right-side panel displaying selected track's tactical data: Track ID, IFF classification, altitude (flight level), azimuth, range, speed, heading. Military-style green Courier text. | 1.7 |
| 1.9 | **HUD Battery Status Panel** | Display all 6 batteries with designation, status (RDY/TRK/ENG/RLD), and missile counts. Highlight available batteries for selected track. | 1.8 |
| 1.10 | **Keyboard Fire Control** | Keys 1-6 assign a battery to the selected track. F key fires. A key aborts. Visual feedback on battery assignment. | 1.9 |

## Tier 2: Core Gameplay Loop (Must Have)

| # | Feature | Description | Depends On |
|---|---------|-------------|------------|
| 2.1 | **Missile Launch Visualization** | Animate missile launch from battery position toward target. Missile trail rendered as a dotted/dashed line on radar scope. | 1.10 |
| 2.2 | **Engagement Result Visualization** | Flash/burst effect on radar scope when missile reaches target. "SPLASH" for kills, "MISS" indicator for misses. | 2.1 |
| 2.3 | **Score Display and Tracking** | Persistent score counter on HUD. Score popup animations (+500, -1000) at event locations. | 1.8 |
| 2.4 | **Message Log / Engagement Feed** | Scrolling text feed on HUD showing: new contacts, IFF results, engagement commands, kill/miss reports, warnings. | 1.8 |
| 2.5 | **Game Over Condition & Screen** | Game over when score drops below threshold or too many hostiles penetrate. Game over overlay with final score and option to restart. | 2.3 |
| 2.6 | **Level Progression** | Automatic level advancement based on score thresholds or time survived. Visual indicator of current level. Level transition announcement. | 2.5 |
| 2.7 | **Hostile Penetration Warning** | Alert when a hostile aircraft reaches the defense zone center. Screen flash, alarm message. Score penalty applied. | 1.4 |
| 2.8 | **Friendly Fire Warning** | Prominent warning when a friendly aircraft is destroyed. Screen flash red, large warning text. Heavy score penalty. | 2.2 |

## Tier 3: Visual Polish & Realism (Should Have)

| # | Feature | Description | Depends On |
|---|---------|-------------|------------|
| 3.1 | **Radar Noise / Clutter** | Random low-intensity green dots scattered across scope simulating ground clutter and atmospheric noise. Intensity adjustable by level. | 1.3 |
| 3.2 | **Track History Trail** | Dotted line behind each blip showing its last N positions. Shows aircraft trajectory. Fades with age. | 1.4 |
| 3.3 | **Battery Position Icons** | Small icons on radar scope showing each battery's position. Color changes with status (green=ready, yellow=reloading, red=engaged). | 1.2 |
| 3.4 | **Phosphor Burn-in Effect** | Stationary objects (range rings, labels) have a slightly brighter persistent glow. Fast-moving targets leave shorter trails than slow ones. | 1.3 |
| 3.5 | **CRT Screen Effect Shader** | Post-processing shader: slight screen curvature, scanlines, vignette, subtle green color bleed. Makes the display look like a real CRT. | 1.2 |
| 3.6 | **Explosion Particles** | Particle effects for missile detonation. Varies by hit/miss. Debris particles for destroyed aircraft. | 2.2 |
| 3.7 | **Console Bezel / Frame** | Graphical border around the radar scope resembling the AN/TSQ-73 console housing. Control panel aesthetic below the scope. | 1.2 |
| 3.8 | **Scope Brightness Pulsing** | Subtle overall brightness variation on the scope simulating CRT power fluctuations. More pronounced at higher difficulty. | 1.3 |

## Tier 4: Audio (Should Have)

| # | Feature | Description | Depends On |
|---|---------|-------------|------------|
| 4.1 | **Radar Sweep Tone** | Subtle ticking/pinging sound synced with sweep beam rotation. Classic radar sound. | 1.3 |
| 4.2 | **New Contact Alert** | Audio alert when a new aircraft enters radar range. Short beep or tone. | 1.4 |
| 4.3 | **IFF Response Tone** | Distinct tones for HOSTILE (warning buzz), FRIENDLY (confirmation beep), UNKNOWN (question tone). | 1.5 |
| 4.4 | **Missile Launch Sound** | Rocket ignition sound when a missile is fired. Doppler effect as it travels. | 2.1 |
| 4.5 | **Explosion / Impact Sound** | Explosion audio for hits. Whoosh/silence for misses. | 2.2 |
| 4.6 | **Warning Klaxon** | Alarm sound for hostile penetration, friendly fire, low missile count. Escalating urgency. | 2.7, 2.8 |
| 4.7 | **Ambient Console Hum** | Low background hum simulating electronics and cooling fans. Immersive ambiance. | 1.1 |

## Tier 5: Advanced Gameplay (Nice to Have)

| # | Feature | Description | Depends On |
|---|---------|-------------|------------|
| 5.1 | **Electronic Countermeasures (ECM)** | Hostile aircraft can jam radar, causing blip to flicker or split into ghost returns. Player must distinguish real from false. | 1.4 |
| 5.2 | **SATCOM Intelligence Updates** | Periodic text messages providing intel on incoming raids: "SIGINT reports bomber formation inbound from NW." Gives player advance warning. | 2.4 |
| 5.3 | **Battery Relocation** | Ability to relocate batteries between engagements. Battery goes offline during move. Strategic positioning decisions. | 1.9 |
| 5.4 | **Missile Resupply** | Resupply convoy arrives periodically. Player must protect it. Replenishes depleted batteries. | 2.6 |
| 5.5 | **Multi-Target Engagement** | Assign multiple batteries to the same high-priority target for increased kill probability. Coordinated salvo. | 1.10 |
| 5.6 | **Aircraft Formation Raids** | Multiple hostile aircraft arriving in coordinated formations (V-formation bombers, fighter escort). | Aircraft system |
| 5.7 | **Terrain Masking** | Low-flying aircraft can hide behind terrain features, appearing and disappearing from radar. | 1.4 |
| 5.8 | **Night/Day Cycle Visual** | Scope aesthetics shift slightly with time of day. Dawn/dusk raid scenarios. | 3.5 |

## Tier 6: Metagame & UI (Nice to Have)

| # | Feature | Description | Depends On |
|---|---------|-------------|------------|
| 6.1 | **Main Menu** | Title screen with game title, Start Game, Settings, High Scores, Quit. Military-themed design. | 1.1 |
| 6.2 | **Settings Screen** | Difficulty selection, audio volume, key bindings, display options. | 6.1 |
| 6.3 | **High Score Table** | Persistent local high scores. Name entry. Top 10 list. | 2.5 |
| 6.4 | **Pause Menu** | In-game pause with resume, restart, settings, quit to menu. | 6.1 |
| 6.5 | **Mission Briefing** | Pre-game screen describing the scenario: expected threat types, available assets, mission objectives. | 6.1 |
| 6.6 | **After Action Report** | Post-game summary: hostiles destroyed, friendlies lost, missiles fired, accuracy %, grade (A-F). | 2.5 |
| 6.7 | **Tutorial / Training Mode** | Guided first-game experience: highlights contacts, explains IFF, walks through engagement. Slower pace. | All Tier 2 |
| 6.8 | **Campaign Mode** | Series of escalating missions with narrative context. Unlock new levels/scenarios. | 6.5, 6.6 |

---

## Priority Legend

| Tier | Priority | Gate |
|------|----------|------|
| Tier 1 | **CRITICAL** | Required for first playable build |
| Tier 2 | **CRITICAL** | Required for complete gameplay loop |
| Tier 3 | **HIGH** | Required for graphical realism target |
| Tier 4 | **HIGH** | Required for immersive experience |
| Tier 5 | **MEDIUM** | Deepens tactical gameplay |
| Tier 6 | **MEDIUM** | Completes the product |
