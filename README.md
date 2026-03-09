# Missile Battery Command

## AN/TSQ-73 Mobile Missile Minder — Air Defense Artillery Battalion Simulator

A tactical air defense game built with cocos2d-x and C++17, simulating a mobile AN/TSQ-73 Missile Minder Battery. You are an Air Defense Artillery (ADA) operator commanding an AN/TSQ-73 console with two missile tracking scopes — Scope 1 (primary PPI radar) and Scope 2 (threat board). Your mission: protect your airspace from waves of incoming hostile aircraft using Patriot, Hawk, and Javelin surface-to-air missile systems while managing a mobile battalion HQ that can relocate when discovered by enemy intelligence.

### Summary

Missile Battery Command simulates the experience of operating an AN/TSQ-73 Missile Minder console in a mobile battalion configuration. The game features:

- **Raytheon AN/TPS-43E surveillance radar** — 250 NM (463 km) range with Plan Position Indicator (PPI) scope
- **Dual tracking scopes** — Scope 1 primary radar display, Scope 2 threat board showing top 5 threats
- **12 missile batteries** — 3 Patriot (MIM-104), 3 Hawk (MIM-23), 3 Javelin (FGM-148) MANPADS, plus 3 loaders per Hawk battery
- **IFF Mode 4 interrogation** — contacts classified as HOSTILE, FRIENDLY, or UNKNOWN with configurable error rates
- **Mobile Battalion HQ** — 90-second relocation cycle (30s teardown + 30s move + 30s setup) with phased radar/comms recovery
- **Airspace management** — no-fly zones, air corridors, restricted areas with altitude bands
- **358 automated tests** across 13 test suites with an interactive ncurses test harness
- **JSON-driven test manifest** — test suite organization managed via `tests/test_manifest.json`

### How to Play

You operate the AN/TSQ-73 Missile Minder console. The PPI radar scope shows all detected aircraft as color-coded blips. Your job is to:

1. **Monitor the radar** — Watch for new contacts appearing at the edge of the 250 NM radar range
2. **Classify threats** — IFF Mode 4 automatically interrogates contacts. Watch for HOSTILE (red), FRIENDLY (blue), and UNKNOWN (yellow) classifications
3. **Check the threat board** — Scope 2 displays the 5 highest-priority threats ranked by proximity, speed, inbound status, and classification
4. **Select a track** — Click a blip on the radar to select it and view its full tactical data (altitude, azimuth, range, speed, heading)
5. **Assign a battery** — Press 1-3 for Patriot, 4-6 for Hawk, 7-9 for Javelin to assign a battery to the selected track
6. **Authorize fire** — Press F to fire. The missile flies to the target with realistic flight time
7. **Assess results** — SPLASH (hit) or MISS. Manage your limited missile supply and reload times
8. **Relocate when needed** — If enemy intelligence discovers your position, order the battalion to relocate

**Scoring:** Destroy hostiles (+100 to +500 by threat level), avoid friendly fire (-1000), prevent hostile penetration (-200), conserve missiles (-25 per miss).

### The Console

Your AN/TSQ-73 displays a Plan Position Indicator (PPI) radar scope — a circular green phosphor display with a rotating sweep beam powered by a Raytheon AN/TPS-43E long-range surveillance radar (250 NM range). As the beam rotates, radar returns (blips) appear at the positions of detected aircraft. Each contact is automatically interrogated using IFF (Identification Friend or Foe) Mode 4. Contacts transition from PENDING to HOSTILE, FRIENDLY, or UNKNOWN based on their IFF response.

The Scope 2 threat board provides a heads-up display of the 5 highest-priority threats, ranked by a composite score factoring proximity (3x weight), speed, inbound determination, hostile classification, altitude, and territory proximity.

Every detected contact is assigned a Track ID (e.g., TK-001) with full tactical data: altitude (flight levels), azimuth (degrees), range (km/NM), speed (knots), and heading.

### Your Arsenal

| Battery | Type | Designation | Range | Ceiling | Missiles | Tracking Radar | Best Against |
|---------|------|-------------|-------|---------|----------|----------------|--------------|
| Mobile Patriot (MPMB) | SAM | PATRIOT-1, -2, -3 | 160 km | FL800 | 4 per launcher | AN/MPQ-53 | Bombers, high-altitude |
| Hawk (HSAMB) | SAM | HAWK-1, -2, -3 | 45 km | FL450 | 3 per launcher + 3 loaders | AN/MPQ-46 HPI | Fighters, drones, low-altitude |
| Javelin (MANPADS) | IR SAM | JAVELIN-1, -2, -3 | 4.75 km | FL015 | 6 per team | IR Seeker | Close-range, low-altitude |

### Threat Matrix

| Aircraft Type | Speed | Altitude Range | Radar Signature | Threat Level |
|---------------|-------|----------------|-----------------|--------------|
| Strategic Bomber | 400-600 kts | FL250-FL450 | Large | HIGH |
| Fighter/Attack | 500-900 kts | FL050-FL350 | Medium | HIGH |
| Tactical Bomber | 350-500 kts | FL150-FL300 | Medium-Large | MEDIUM |
| Recon Drone (UAV) | 100-200 kts | FL100-FL500 | Small | LOW |
| Attack Drone (UCAV) | 150-300 kts | 500ft-FL150 | Small | MEDIUM |
| Stealth Fighter | 500-800 kts | FL150-FL400 | Very Small | CRITICAL |
| Civilian Airliner | 400-500 kts | FL300-FL400 | Large | FRIENDLY |
| Friendly Military | 250-600 kts | FL100-FL400 | Medium | FRIENDLY |

### Game Difficulty

Five escalating levels control the intensity of the air battle:

- **Level 1** — Training: 3 max contacts, slow spawn, no stealth, no IFF errors
- **Level 2** — Alert: 5 max contacts, moderate spawn, no stealth
- **Level 3** — Engaged: 8 max contacts, occasional IFF errors
- **Level 4** — Under Fire: 10 max contacts, stealth aircraft appear, IFF becoming unreliable
- **Level 5** — Overwhelmed: 15 max contacts, rapid spawn, frequent IFF errors, fast targets

### Scoring

| Event | Points |
|-------|--------|
| Hostile destroyed | +100 to +500 (by threat level) |
| Friendly destroyed | -1000 |
| Hostile penetrates defense zone | -200 |
| Missile wasted (miss) | -25 |
| First-shot kill bonus | 2x multiplier |

### Building

**Prerequisites:** CMake 3.10+, C++17 compiler, cocos2d-x (optional for full graphics), ncurses (optional for test harness)

```bash
# Clone
git clone <repo-url>
cd Missile-Battery-Command

# Build (stub mode — console simulation, no cocos2d-x required)
mkdir build && cd build
cmake ..
make -j$(nproc)
./MissileBatteryCommand

# Build (full graphics mode — requires cocos2d-x)
cmake -DCOCOS2DX_ROOT=/path/to/cocos2d-x ..
make -j$(nproc)
./MissileBatteryCommand

# Run tests
./MissileBatteryCommandTests

# Run interactive test harness (requires ncurses)
./TestHarness

# Run tests in batch mode (CI-friendly)
./TestHarness --batch
```

The stub mode runs a 60-second console simulation showing aircraft spawning, IFF classification, and live track tables. When linked with cocos2d-x, the full radar PPI scope renders with the green sweep beam, colored blips, battery positions, and the complete HUD.

### Project Structure

```
Missile-Battery-Command/
├── CMakeLists.txt                 # Build system (auto-detects cocos2d-x)
├── README.md                      # This file
├── docs/
│   ├── DEVELOPMENT_PLAN.md        # Detailed game design document
│   ├── DEVELOPMENT_LOG.md         # Commit-by-commit progress log
│   ├── BACKLOG.md                 # Prioritized feature backlog
│   └── PLAN.md                    # Phased development roadmap
├── include/
│   ├── GameTypes.h                # Enums, structs, constants
│   ├── GameConfig.h               # Difficulty levels and tuning
│   ├── AppDelegate.h              # Application entry point
│   ├── RadarScene.h               # Main game scene orchestrator
│   ├── RadarDisplay.h             # PPI radar scope rendering
│   ├── Aircraft.h                 # Aircraft entity (8 types)
│   ├── AircraftGenerator.h        # Level-scaled random spawner
│   ├── IFFSystem.h                # Mode 4 friend/foe identification
│   ├── TrackManager.h             # Track ID assignment and updates
│   ├── MissileBattery.h           # Patriot, Hawk, and Javelin battery logic
│   ├── FireControlSystem.h        # Battery management and engagement
│   ├── GameHUD.h                  # Track info, battery status, score
│   ├── AirspaceManager.h          # Airspace zones (NFZ, corridors, restricted)
│   ├── DataCard.h                 # Equipment and troop data cards
│   ├── ThreatBoard.h              # Scope 2 threat ranking display
│   └── BattalionHQ.h              # Mobile battalion HQ relocation
├── src/
│   ├── main.cpp                   # Entry point
│   ├── AppDelegate.cpp            # App lifecycle
│   ├── RadarScene.cpp             # Game loop and scene management
│   ├── RadarDisplay.cpp           # Radar rendering with DrawNode
│   ├── Aircraft.cpp               # Aircraft movement and properties
│   ├── AircraftGenerator.cpp      # Spawn logic with weighted types
│   ├── IFFSystem.cpp              # Interrogation with error modeling
│   ├── TrackManager.cpp           # Real-time track data management
│   ├── MissileBattery.cpp         # Engagement and reload simulation
│   ├── FireControlSystem.cpp      # Multi-battery coordination
│   ├── GameConfig.cpp             # Level configuration tables
│   ├── GameHUD.cpp                # HUD panel rendering
│   ├── AirspaceManager.cpp        # Airspace zone management
│   ├── DataCard.cpp               # Equipment/troop card generation
│   ├── ThreatBoard.cpp            # Threat scoring and ranking
│   └── BattalionHQ.cpp            # HQ relocation state machine
├── tests/
│   ├── TestFramework.h            # Custom test framework (no dependencies)
│   ├── test_main.cpp              # Test runner entry point
│   ├── test_manifest.json         # JSON test suite definitions (for harness)
│   ├── test_harness.cpp           # Interactive ncurses test harness
│   ├── test_cocos2dx_validation.cpp
│   ├── test_game_types.cpp
│   ├── test_aircraft.cpp
│   ├── test_iff_system.cpp
│   ├── test_track_manager.cpp
│   ├── test_missile_battery.cpp
│   ├── test_aircraft_generator.cpp
│   ├── test_fire_control.cpp
│   ├── test_game_config.cpp
│   ├── test_airspace_manager.cpp
│   ├── test_data_card.cpp
│   ├── test_threat_board.cpp
│   └── test_battalion_hq.cpp
└── Resources/                     # Game assets (future)
```

### Current Status

The game framework is fully operational with 16 core systems, 358 tests across 13 suites, and a mobile battalion HQ configuration. The AN/TSQ-73 operates as a mobile Missile Minder HQ Battery with Raytheon AN/TPS-43E long-range surveillance radar, dual tracking scopes, Patriot/Hawk/Javelin missile systems, IFF Mode 4, airspace management, equipment data cards, and battalion relocation capability. Development is in stub mode (console simulation); the next major milestone is integrating cocos2d-x for the graphical PPI scope.

### Development Roadmap

Development follows 7 phases, each building on the last. See [PLAN.md](docs/PLAN.md) for full details and exit criteria.

| Phase | Description | Status |
|-------|-------------|--------|
| **Phase 1** | Foundation — C++ framework, all core systems, console stub simulation | COMPLETE |
| **Phase 2** | Graphical Radar Console — cocos2d-x PPI scope, sweep beam, blips, track selection, battery icons | COMPLETE |
| **Phase 3** | Interactive Fire Control — HUD panels, keyboard engagement flow, missile flight visualization, hit/miss results | Planned |
| **Phase 4** | Visual Realism — CRT shader (barrel distortion, scanlines, vignette), phosphor effects, radar noise/clutter, track trails, console bezel, explosion particles | Planned |
| **Phase 5** | Audio Design — ambient console hum, radar sweep tones, IFF response sounds, missile launch/impact, warning klaxons | Planned |
| **Phase 6** | Metagame & Polish — main menu, settings, mission briefings, after action reports, high scores, tutorial, campaign mode | Planned |
| **Phase 7** | Advanced Tactics — ECM/jamming, formation raids, terrain masking, battery relocation, resupply convoys, SIGINT intel feed | Stretch |

Phases 4 and 5 can be developed in parallel after Phase 3. Phase 7 items are independent and can be added incrementally.

### Feature Backlog

Over 40 features are tracked and prioritized across 6 tiers. See [BACKLOG.md](docs/BACKLOG.md) for the full list with dependencies.

| Tier | Priority | Focus | Items |
|------|----------|-------|-------|
| **Tier 1** | CRITICAL | Graphical Radar Console — cocos2d-x build, PPI scope rendering, sweep beam, blip color-coding, track labels/selection, HUD panels, keyboard fire control | 10 |
| **Tier 2** | CRITICAL | Core Gameplay Loop — missile launch visualization, engagement results, score tracking, message log, game over screen, level progression, penetration/friendly-fire warnings | 8 |
| **Tier 3** | HIGH | Visual Polish — radar noise/clutter, track history trails, battery position icons, phosphor burn-in, CRT screen shader, explosion particles, console bezel, scope brightness pulsing | 8 |
| **Tier 4** | HIGH | Audio — radar sweep tone, new contact alert, IFF response tones, missile launch/explosion sounds, warning klaxon, ambient console hum | 7 |
| **Tier 5** | MEDIUM | Advanced Gameplay — ECM, SATCOM intelligence, battery relocation, missile resupply, multi-target engagement, formation raids, terrain masking, day/night cycle | 8 |
| **Tier 6** | MEDIUM | Metagame & UI — main menu, settings screen, high score table, pause menu, mission briefing, after action report, tutorial mode, campaign mode | 8 |
