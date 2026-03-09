# Development Plan

## Missile Battery Command — Phased Roadmap to Graphically Realistic Game

---

## Phase 1: Foundation (COMPLETE)

**Goal:** Establish the full C++ game architecture, all core systems, and a working simulation.

**Deliverables:**
- [x] CMake build system with cocos2d-x auto-detection and stub fallback
- [x] `GameTypes.h` — All enumerations, structs, and game constants
- [x] `Aircraft` — 8 aircraft types with realistic performance envelopes
- [x] `AircraftGenerator` — Weighted random spawner, difficulty-scaled
- [x] `IFFSystem` — Mode 4 interrogation with timing and error modeling
- [x] `TrackManager` — Track ID assignment and real-time data updates
- [x] `MissileBattery` — Patriot (MPMB) and Hawk (HSAMB) with engagement simulation
- [x] `FireControlSystem` — 6-battery management, target assignment, engagement coordination
- [x] `RadarDisplay` — PPI scope rendering (sweep beam, range rings, blips, phosphor fade)
- [x] `GameHUD` — Track info panel, battery status, score, message log
- [x] `GameConfig` — 5 difficulty levels with full parameter tuning
- [x] `RadarScene` — Game loop, input handling, scoring, game over
- [x] `AppDelegate` / `main.cpp` — Application lifecycle and entry point
- [x] Console stub simulation verified (60-second run with track tables)

**Status:** COMPLETE — All 12 classes implemented, compiles clean, simulation verified.

---

## Phase 2: Graphical Radar Console (COMPLETE)

**Goal:** Render the AN/TSQ-73 PPI radar scope as a graphically realistic, interactive display using cocos2d-x.

**Prerequisites:** cocos2d-x 4.0 installed and linked.

### Step 2.1 — cocos2d-x Build Integration
- [x] Link cocos2d-x via CMake
- [x] Verify window creation at 1280x720 design resolution
- [x] Render a blank black scene to confirm the graphics pipeline works
- [x] Test on target platform (Linux/macOS/Windows)

### Step 2.2 — Radar Scope Background
- [x] Draw the circular PPI scope (dark green/black background)
- [x] Render 5 range rings labeled in nautical miles (Raytheon AN/TPS-43E 250 NM range)
- [x] Draw 12 azimuth lines at 30-degree intervals
- [x] Label cardinal directions (N/S/E/W) and degree markings
- [x] Add center crosshair marking the radar/defense zone position

### Step 2.3 — Rotating Sweep Beam
- [x] Implement the sweep beam as a filled triangle wedge with 30° phosphor trail
- [x] Rotate clockwise at 6 RPM (36 degrees/second)
- [x] Non-linear phosphor brightness decay (`pow(fadeRatio, 0.6f)`)
- [x] 20-segment gradient for smooth trailing fade

### Step 2.4 — Radar Blips
- [x] Render detected aircraft as filled circles on the scope
- [x] Blip size proportional to radar cross-section
- [x] Phosphor brightness tied to sweep timing
- [x] Color by IFF status: red (hostile), blue (friendly), yellow (unknown), gray (pending)
- [x] Glow effect for bright blips

### Step 2.5 — Track Labels and Selection
- [x] Track ID text (e.g., "TK-003") adjacent to each blip
- [x] Click/touch to select a track via `findNearestTrack()` with proper coordinate conversion
- [x] Animated pulsing corner bracket + ring highlight on selected track
- [x] Deselect when clicking empty space

### Step 2.6 — Battery Position Indicators
- [x] Battery icons at positions on the scope (P1-P3 Patriot, H1-H3 Hawk)
- [x] Status-coded colors (green=ready, amber=reloading, red=engaged, gray=offline)
- [x] Labeled with short designation

### Additional Phase 2 Features
- [x] Track history trails (8 points, 1.5s intervals, 15s max age)
- [x] Radar noise/ground clutter (120 dots, quadratic radial distribution)
- [x] Dashed territory defense zone ring
- [x] Missile flight trails (dashed cyan)
- [x] Enhanced HUD: range bands, available batteries, reload timers, NM+km display
- [x] Raytheon AN/TPS-43E radar upgrade (250 NM / 463 km)
- [x] All 212 tests passing

**Status:** COMPLETE — Full PPI scope with sweep beam, blips, trails, noise, selection, batteries, and HUD. All constants updated for Raytheon AN/TPS-43E long-range radar.

---

## Phase 2.5: Tactical Systems Expansion (COMPLETE)

**Goal:** Expand the tactical simulation with additional weapon systems, airspace management, threat assessment, and mobile operations.

**Deliverables:**
- [x] Javelin MANPADS (FGM-148) — 3 platoons, 4.75 km range, IR seeker, 6 missiles per team
- [x] Hawk ammunition logistics — 9 missile total stock, 3 loaders per battery
- [x] Tracking radar modeling — AN/MPQ-53 (Patriot), AN/MPQ-46 HPI (Hawk), IR Seeker (Javelin)
- [x] Battery engagement statistics — hit/miss/engagement counts
- [x] Airspace management — NFZ, air corridors, restricted areas with azimuth/range/altitude parameters
- [x] Equipment data cards — per-weapon-system equipment, troop strength, and intel summary cards
- [x] Battery relocation — configurable relocation time per battery type, OFFLINE during move
- [x] Threat Board (Scope 2) — top 5 threats with composite scoring (proximity 3x, speed, inbound, classification)
- [x] Mobile Battalion HQ — 90s relocation cycle (30s teardown + 30s move + 30s setup), phased recovery
- [x] Game HUD integration — threat board panel, HQ status panel
- [x] Interactive ncurses test harness — suite browsing, test enable/disable, search, batch mode
- [x] JSON test manifest — test suite definitions in `test_manifest.json` for easy maintenance
- [x] 358 tests across 13 suites — all passing

**Status:** COMPLETE — Full tactical expansion with 16 core systems and comprehensive test coverage.

---

## Phase 3: Interactive Fire Control

**Goal:** Full interactive engagement workflow — from track selection through missile impact.

### Step 3.1 — HUD Integration
- Track info panel (right side): selected track's full tactical data
- Battery status panel: all 6 batteries with status, missile count
- Highlight batteries that can engage the selected track (in range + altitude)
- Score and level display, top-right corner
- Green-on-black Courier font, military aesthetic

### Step 3.2 — Keyboard/Mouse Engagement Flow
- Select track on radar (click)
- Available batteries highlight on battery panel
- Press 1-6 to assign battery to selected track
- Battery status changes to TRACKING
- Press F to authorize fire — missile launches
- Press A to abort engagement before firing
- Visual confirmation of each step in message log

### Step 3.3 — Missile Flight Visualization
- Missile trail rendered on radar scope from battery to target
- Trail drawn as a moving dot with a fading line behind it
- Flight time proportional to distance (Mach 3 Patriot, Mach 2.5 Hawk)
- Trail color: white or cyan to distinguish from aircraft blips

### Step 3.4 — Engagement Results
- Hit: brief flash/burst at target location, blip disappears, "SPLASH" in message log
- Miss: missile trail fades out past the target, "MISS" in message log
- Score popup at engagement location (+500, -1000, etc.)
- Battery transitions to RELOADING with countdown

### Step 3.5 — Warnings and Alerts
- Hostile penetration: scope border flashes red, alarm text, score penalty
- Friendly fire: full-screen red flash, "FRIENDLY FIRE" warning, heavy penalty
- Low missiles: battery status flashes, warning message
- Multiple simultaneous threats: "RAID WARNING" when 3+ hostiles in same sector

**Exit Criteria:** Complete engagement loop is playable — select, assign, fire, observe result. Player can defend the airspace through pure radar-based tactical decisions.

---

## Phase 4: Visual Realism

**Goal:** Make the radar display look like an authentic military CRT console.

### Step 4.1 — CRT Post-Processing Shader
- Fragment shader applied as a full-screen post-process
- Slight barrel distortion (CRT screen curvature)
- Horizontal scanlines (subtle, 1-pixel dark bands)
- Edge vignetting (darkening toward corners)
- Green color channel emphasis with slight bleed
- Subtle RGB chromatic aberration at edges

### Step 4.2 — Phosphor Effects
- Persistent glow on static elements (range rings, text)
- Blip brightness directly tied to time-since-last-sweep
- Fast aircraft leave shorter phosphor trails than slow ones
- Occasional bloom on bright events (missile launch, explosion)

### Step 4.3 — Radar Noise and Clutter
- Random low-intensity green dots scattered across scope (atmospheric noise)
- Denser noise near center (ground clutter simulation)
- Noise intensity scales with difficulty level
- Noise refreshes each sweep, never identical pattern

### Step 4.4 — Track History Trails
- Dotted trail behind each blip showing last 5-10 positions
- Trail dots spaced at sweep intervals
- Oldest positions most faded
- Trail clearly shows direction and speed of movement
- Trail disappears after aircraft is destroyed

### Step 4.5 — Console Frame / Bezel
- Graphical bezel around the radar scope resembling the AN/TSQ-73 housing
- Metal/dark gray textured frame with rivets or panel lines
- Control panel area below scope (status lights, designation plates)
- Adds to immersion — the scope is embedded in a physical console

### Step 4.6 — Explosion and Impact Effects
- Particle system for missile detonation at target location
- Hit: orange/white starburst expanding and fading, debris particles
- Miss: smaller puff, missile trail dissipates
- Ground impact effect if missile misses and continues past
- Particle colors visible against the green radar background

**Exit Criteria:** The display is visually indistinguishable from a real CRT radar console at a glance. Phosphor glow, CRT distortion, noise, and particle effects create an authentic atmosphere.

---

## Phase 5: Audio Design

**Goal:** Immersive military console audio environment.

### Step 5.1 — Ambient Sounds
- Background electronics hum (constant, low volume)
- Cooling fan white noise
- Occasional relay click sounds

### Step 5.2 — Radar Audio
- Sweep beam tick/ping synced with each rotation
- New contact alert tone (short ascending beep)
- IFF response tones: hostile (warning buzz), friendly (confirmation ding), unknown (questioning tone)

### Step 5.3 — Engagement Audio
- Missile launch: rocket ignition burst
- Missile in flight: faint whoosh (increasing as it nears target)
- Hit: explosion thud
- Miss: fading whoosh

### Step 5.4 — Warning Audio
- Hostile penetration klaxon
- Friendly fire alarm (distinct from penetration)
- Low missile warning tone
- Game over siren

**Exit Criteria:** Playing with eyes closed, the audio alone conveys the experience of sitting at a radar console during an air defense engagement.

---

## Phase 6: Metagame and Polish

**Goal:** Complete game product with menus, progression, and replayability.

### Step 6.1 — Main Menu
- Title screen: "MISSILE BATTERY COMMAND" with military stencil font
- Animated radar scope in background
- Options: New Game, Continue, Settings, High Scores, Quit
- Military-themed UI design (green/black, angular borders)

### Step 6.2 — Settings
- Difficulty level selection
- Audio volume controls (master, SFX, ambient)
- Display options (CRT shader on/off, fullscreen/windowed)
- Key bindings customization

### Step 6.3 — Mission Briefing
- Pre-game screen between menu and gameplay
- Scenario description: expected threat level, special conditions
- Intel on likely threat types
- Battery deployment overview
- "COMMENCE OPERATIONS" button to start

### Step 6.4 — After Action Report
- Post-game summary screen
- Statistics: hostiles destroyed, friendlies lost, missiles fired, accuracy %
- Performance grade (A through F)
- Score breakdown by category
- Option to replay or return to menu

### Step 6.5 — High Score System
- Local persistent high score table (top 10)
- Name entry for new high scores
- Score, level reached, date
- Displayed from main menu

### Step 6.6 — Tutorial Mode
- Guided first-play experience
- Highlighted UI elements with explanatory text
- Slowed-down gameplay for learning
- Covers: radar reading, IFF, track selection, battery assignment, engagement

### Step 6.7 — Campaign Mode (Stretch Goal)
- Series of 10 escalating missions
- Narrative context: "Day 1 — First Contact", "Day 5 — The Night Raid", etc.
- Special conditions per mission (fog, ECM, limited batteries)
- Unlockable missions based on performance

**Exit Criteria:** A complete, polished game product that can be launched, played through multiple sessions, and provides a satisfying tactical air defense experience.

---

## Phase 7: Advanced Tactical Features (Stretch Goals)

These items deepen the tactical simulation if development bandwidth allows.

### Step 7.1 — Electronic Countermeasures (ECM)
- Hostile aircraft can jam radar
- Blips flicker, split into ghosts, or temporarily disappear
- Player must track through jamming using track history and prediction
- ECM aircraft are high-priority targets

### Step 7.2 — Formation Raids
- Multiple hostiles arriving in coordinated formations
- V-formation bomber groups with fighter escorts
- Requires prioritizing the escort vs. the bombers
- Formation aircraft share a raid designation (RAID-01)

### Step 7.3 — Terrain Masking
- Low-altitude aircraft can drop below radar coverage
- Terrain profile drawn on scope (mountain ridges block LOS)
- Aircraft disappear when masked, reappear on the other side
- Hawk batteries are better against terrain-masking targets (lower coverage)

### Step 7.4 — Resupply and Battery Relocation
- Ammunition resupply convoys that arrive periodically
- Player can reposition batteries (go offline during move) — **DONE (battery relocation)**
- Strategic battery placement matters as threats shift

### Step 7.5 — SIGINT Intelligence Feed
- Text messages providing advance warning of raids
- "SIGINT: BOMBER FORMATION FORMING UP, SECTOR NW, ETA 120 SECONDS"
- Allows pre-positioning defensive focus

**Exit Criteria:** The game offers deep tactical decisions beyond simple point-and-shoot, with ECM, formations, terrain, and logistics creating a rich simulation.

---

## Timeline Summary

| Phase | Description | Builds On | Status |
|-------|-------------|-----------|--------|
| 1 | Foundation (C++ framework) | — | COMPLETE |
| 2 | Graphical Radar Console | Phase 1 | COMPLETE |
| 2.5 | Tactical Systems Expansion | Phase 2 | COMPLETE |
| 3 | Interactive Fire Control | Phase 2 | Next up |
| 4 | Visual Realism | Phase 3 | Planned |
| 5 | Audio Design | Phase 3 | Planned |
| 6 | Metagame & Polish | Phase 3-5 | Planned |
| 7 | Advanced Tactics | Phase 6 | Stretch |

Phases 4 and 5 can be developed in parallel once Phase 3 is complete.
Phase 6 can begin its menu/UI work during Phase 4-5 development.
Phase 7 items are independent and can be added incrementally at any time after Phase 3.
