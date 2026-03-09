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

## Phase 2: Graphical Radar Console

**Goal:** Render the AN/TSQ-73 PPI radar scope as a graphically realistic, interactive display using cocos2d-x.

**Prerequisites:** cocos2d-x 4.0 installed and linked.

### Step 2.1 — cocos2d-x Build Integration
- Link cocos2d-x via CMake
- Verify window creation at 1280x720 design resolution
- Render a blank black scene to confirm the graphics pipeline works
- Test on target platform (Linux/macOS/Windows)

### Step 2.2 — Radar Scope Background
- Draw the circular PPI scope (dark green/black background)
- Render 5 range rings at 20 km intervals using `DrawNode`
- Label each ring with distance (20, 40, 60, 80, 100 km)
- Draw 12 azimuth lines at 30-degree intervals
- Label cardinal directions (N/S/E/W) and degree markings
- Add center crosshair marking the radar/defense zone position

### Step 2.3 — Rotating Sweep Beam
- Implement the sweep beam as a triangular gradient wedge
- Rotate clockwise at 6 RPM (36 degrees/second)
- Leading edge: bright green, opaque
- Trailing edge: gradient fade to transparent over ~30 degrees
- Subtle glow effect on the leading edge using additive blending

### Step 2.4 — Radar Blips
- Render detected aircraft as small filled circles on the scope
- Blip size proportional to radar cross-section (large bombers = bigger blips)
- Blips illuminate (peak brightness) when the sweep beam passes
- Phosphor decay: blips gradually fade between sweeps (~85% intensity by next pass)
- Color by IFF status: red (hostile), blue (friendly), yellow (unknown), gray (pending)
- Smooth color transitions when IFF status changes

### Step 2.5 — Track Labels and Selection
- Render Track ID text (e.g., "TK-003") adjacent to each blip
- Offset labels to prevent overlap with the blip dot
- Click/touch to select a track — nearest blip within threshold
- Selected track: animated bracket or pulsing ring highlight
- Deselect when clicking empty space

### Step 2.6 — Battery Position Indicators
- Small diamond/triangle icons at each battery's position on the scope
- Color-coded: green (ready), amber (reloading), red (engaged), gray (offline)
- Labeled with short designation (P1, P2, P3, H1, H2, H3)

**Exit Criteria:** A graphically rendered radar scope with rotating sweep beam, color-coded blips appearing and fading, selectable tracks, and battery positions visible. The radar should look and feel like a real military PPI display.

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
- Player can reposition batteries (go offline during move)
- Strategic battery placement matters as threats shift

### Step 7.5 — SIGINT Intelligence Feed
- Text messages providing advance warning of raids
- "SIGINT: BOMBER FORMATION FORMING UP, SECTOR NW, ETA 120 SECONDS"
- Allows pre-positioning defensive focus

**Exit Criteria:** The game offers deep tactical decisions beyond simple point-and-shoot, with ECM, formations, terrain, and logistics creating a rich simulation.

---

## Timeline Summary

| Phase | Description | Builds On | Scope |
|-------|-------------|-----------|-------|
| 1 | Foundation (C++ framework) | — | COMPLETE |
| 2 | Graphical Radar Console | Phase 1 | First visual build |
| 3 | Interactive Fire Control | Phase 2 | First playable build |
| 4 | Visual Realism | Phase 3 | Authentic CRT look |
| 5 | Audio Design | Phase 3 | Immersive sound |
| 6 | Metagame & Polish | Phase 3-5 | Complete product |
| 7 | Advanced Tactics | Phase 6 | Deep simulation |

Phases 4 and 5 can be developed in parallel once Phase 3 is complete.
Phase 6 can begin its menu/UI work during Phase 4-5 development.
Phase 7 items are independent and can be added incrementally at any time after Phase 3.
