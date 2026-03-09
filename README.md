# Missile Battery Command

## AN/TSQ-73 Air Defense Artillery Battalion Simulator

A tactical air defense game built with cocos2d-x and C++17. You are an Air Defense Artillery (ADA) operator stationed at the controls of an AN/TSQ-73 missile minder console. Your mission: protect your country's airspace from waves of incoming hostile aircraft — strategic bombers, fighter/attack jets, tactical bombers, reconnaissance drones, attack drones, and stealth fighters — using a combination of Patriot and Hawk surface-to-air missile batteries.

### The Console

Your AN/TSQ-73 displays a Plan Position Indicator (PPI) radar scope — a circular green phosphor display with a rotating sweep beam. As the beam rotates, radar returns (blips) appear at the positions of detected aircraft. Each contact is automatically interrogated using IFF (Identification Friend or Foe) Mode 4. Contacts transition from PENDING to HOSTILE, FRIENDLY, or UNKNOWN based on their IFF response.

Every detected contact is assigned a Track ID (e.g., TK-001) with full tactical data: altitude (flight levels), azimuth (degrees), range (km), speed (knots), and heading.

### Your Arsenal

| Battery | Type | Designation | Range | Ceiling | Missiles | Best Against |
|---------|------|-------------|-------|---------|----------|--------------|
| Mobile Patriot (MPMB) | SAM | PATRIOT-1, -2, -3 | 70 km | FL800 | 4 per launcher | Bombers, high-altitude |
| Hawk (HSAMB) | SAM | HAWK-1, -2, -3 | 40 km | FL450 | 3 per launcher | Fighters, drones, low-altitude |

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

**Prerequisites:** CMake 3.10+, C++17 compiler, cocos2d-x (optional for full graphics)

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
│   ├── MissileBattery.h           # Patriot and Hawk battery logic
│   ├── FireControlSystem.h        # Battery management and engagement
│   └── GameHUD.h                  # Track info, battery status, score
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
│   └── GameHUD.cpp                # HUD panel rendering
└── Resources/                     # Game assets (future)
```

### Current Status

The complete game framework is operational. All core systems — radar display, aircraft generation, IFF interrogation, track management, missile batteries, and fire control — are implemented and verified through the console stub simulation. The next step is integrating cocos2d-x for the graphical radar scope and building out the interactive gameplay loop.

See [PLAN.md](docs/PLAN.md) for the full development roadmap and [BACKLOG.md](docs/BACKLOG.md) for the prioritized feature list.
