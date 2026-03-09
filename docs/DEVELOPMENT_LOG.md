# Development Log

## Missile Battery Command — AN/TSQ-73 Air Defense Simulator

---

### Commit 1: `6c3b92d` — Initial Game Framework

**Date:** 2026-03-09
**Branch:** `claude/aircraft-defense-game-bsd74`
**Summary:** Complete C++ game framework with cocos2d-x integration for an air defense artillery simulator.

#### What Was Built

**Build System (CMakeLists.txt)**
- CMake 3.10+ with C++17 standard
- Auto-detects cocos2d-x installation (searches common paths and environment variable)
- Falls back to stub mode when cocos2d-x is not available, enabling development and testing without the graphics engine
- Compiles all 12 source files and 12 headers

**Core Types and Constants (GameTypes.h)**
- Defined all game enumerations: `AircraftType` (8 types), `IFFStatus`, `BatteryType`, `BatteryStatus`, `EngagementResult`
- `TrackData` struct carrying full track information (ID, classification, altitude, azimuth, range, speed, heading) with formatted string output (flight levels, track ID strings)
- `BatteryData` struct for battery status display
- `GameConstants` namespace with all tuning values: radar range (100 km), sweep rate (6 RPM), IFF interrogation time (2s), Patriot range (70 km / FL800 / 4 missiles), Hawk range (40 km / FL450 / 3 missiles), scoring values

**Radar Display (RadarDisplay.h/.cpp)**
- PPI (Plan Position Indicator) scope rendered via cocos2d-x DrawNode
- Rotating sweep beam with configurable rotation speed (6 RPM = 36 degrees/sec)
- 5 range rings at 20 km intervals with labeled distances
- 12 azimuth lines at 30-degree intervals with degree labels
- Radar blips rendered as colored dots: red (hostile), blue (friendly), yellow (unknown), gray (pending)
- Phosphor fade effect — blips brighten as sweep passes, then decay over time
- Selected track highlight ring
- Battery position indicators on the scope
- Full stub implementation for non-graphical development

**Aircraft System (Aircraft.h/.cpp)**
- 8 aircraft types with realistic performance envelopes (speed, altitude, radar cross-section)
- Position tracking in polar coordinates (azimuth, range from radar site)
- Heading-based movement with per-frame position updates
- Friendly/hostile classification separate from IFF status
- Territory penetration detection (aircraft reaching defense zone center)
- Track data generation for display systems

**Aircraft Generator (AircraftGenerator.h/.cpp)**
- Weighted random aircraft type selection (bombers, fighters, drones, stealth)
- Spawn rate controlled by game level (8-15 second intervals at Level 1, down to 2-5 seconds at Level 5)
- Maximum concurrent aircraft cap per level (3 at Level 1, 15 at Level 5)
- Friendly aircraft mixed in based on level's friendly ratio (30% at Level 1, 10% at Level 5)
- Stealth fighters only appear at Level 4+
- Aircraft spawn at radar edge (85-100 km) at random azimuths heading inward

**IFF System (IFFSystem.h/.cpp)**
- Simulates real Mode 4 IFF interrogation with a 2-second processing delay
- Contacts start as PENDING, transition to HOSTILE, FRIENDLY, or UNKNOWN
- Configurable error rate per difficulty level (0% at Level 1, up to 20% at Level 5)
- Error types: complete misidentification (30% of errors) or inconclusive UNKNOWN (70% of errors)
- Stealth aircraft have 30% chance of returning UNKNOWN even without errors
- Prevents duplicate interrogation of already-processing contacts

**Track Manager (TrackManager.h/.cpp)**
- Assigns sequential Track IDs (TK-001, TK-002, ...) to new radar contacts
- Maintains mapping between track IDs and aircraft entities
- Real-time track data updates every frame
- Queries: all tracks, hostile-only tracks, by track ID
- Track counts by classification (hostile, friendly, total active)
- Automatic IFF interrogation trigger on new tracks

**Missile Battery (MissileBattery.h/.cpp)**
- Two battery types modeled: Patriot (MPMB) and Hawk (HSAMB)
- Patriot: 70 km range, FL800 ceiling, 4 missiles, 8-second reload, positioned at 5 km from center
- Hawk: 40 km range, FL450 ceiling, 3 missiles, 5-second reload, positioned at 3 km from center
- Engagement validation: checks range, altitude, and missile availability
- Kill probability calculation based on weapon-target pairing (0.45-0.85 base probability)
- Missile flight time simulation (Mach 3 for Patriot, Mach 2.5 for Hawk)
- Battery states: READY, TRACKING, ENGAGED, RELOADING, DESTROYED, OFFLINE
- Engagement result reporting (HIT/MISS)

**Fire Control System (FireControlSystem.h/.cpp)**
- Manages all 6 batteries (3 Patriot + 3 Hawk) in a triangular/hexagonal formation
- Patriot batteries at 0, 120, 240 degrees; Hawk batteries at 60, 180, 300 degrees
- Target assignment with validation (can this battery engage this target?)
- Available battery query for a given track (which batteries can reach it?)
- Engagement result collection and reporting
- Full reset capability for new games

**Game HUD (GameHUD.h/.cpp)**
- Track info panel: displays selected track's full tactical data (ID, classification, altitude, azimuth, range, speed, heading)
- Battery status panel: all 6 batteries with designation, status code (RDY/TRK/ENG/RLD/DES/OFF), and missile count
- Score and level display
- Scrolling message log (last 8 messages) for engagement reports and alerts
- All text in military-style Courier green-on-black

**Game Config (GameConfig.h/.cpp)**
- Singleton configuration manager
- 5 difficulty levels with full parameter sets: max aircraft, spawn intervals, stealth toggle, IFF error rate, speed multiplier, friendly ratio, score bonuses
- Per-aircraft-type scoring: Strategic Bomber +500, Fighter +300, Tactical Bomber +250, Attack Drone +200, Recon Drone +100, Stealth Fighter +500

**Main Scene (RadarScene.h/.cpp)**
- Full game loop: spawn → update aircraft → update tracks → update fire control → check engagements → cleanup → check game over
- Touch/click input: select nearest track on radar scope
- Keyboard input: keys 1-6 assign batteries (1-3 Patriot, 4-6 Hawk), F to fire, A to abort
- Engagement result processing with scoring (hostile kill, friendly fire penalty, miss penalty)
- Game over condition: score drops below -2000
- Console stub mode: runs 60-second simulation with formatted track tables every 5 seconds

**Application Entry (main.cpp, AppDelegate.h/.cpp)**
- Standard cocos2d-x application lifecycle
- Stub mode entry point with banner and console simulation
- Design resolution: 1280x720

#### Verification

The project compiles cleanly with GCC 13.3 in stub mode (no cocos2d-x). The 60-second console simulation was run successfully, demonstrating:
- Aircraft spawning at radar edge with random types and azimuths
- IFF interrogation transitioning contacts from PENDING to HOSTILE/FRIENDLY
- Track table display with full tactical data (flight levels, azimuth, range, speed, heading)
- All 6 missile batteries initialized and reporting READY status
- 3 contacts tracked over the simulation period

---

### Commit 2 — Phase 2: Graphical Radar Console & Raytheon AN/TPS-43E Upgrade

**Date:** 2026-03-09
**Branch:** `claude/aircraft-defense-game-bsd74`
**Summary:** Full Phase 2 implementation — graphical PPI radar scope with cocos2d-x, upgraded to Raytheon AN/TPS-43E long-range surveillance radar (250 NM / 463 km).

#### Radar System Upgrade — AN/TPS-43E

The radar model was upgraded from a short-range 100 km system to the Raytheon AN/TPS-43E long-range surveillance radar:
- **Range:** 250 nautical miles (463 km), well beyond earth's curvature
- **Detection:** Pulse-Doppler with MTI for beyond-horizon detection
- All game constants scaled proportionally for the new radar envelope

#### Constants Updated (GameTypes.h)
- `RADAR_MAX_RANGE_KM`: 100 → 463 km (250 NM)
- `RADAR_MAX_RANGE_NM`: Added — 250.0 NM
- `NM_TO_KM` / `KM_TO_NM`: Added unit conversions (1 NM = 1.852 km)
- `PATRIOT_MAX_RANGE`: 70 → 160 km (~86 NM) — MIM-104 realistic range
- `HAWK_MAX_RANGE`: 40 → 45 km (~24 NM) — MIM-23 realistic range
- `TERRITORY_RADIUS_KM`: 10 → 25 km (~13.5 NM) — scaled defense zone

#### Graphical Radar Display (RadarDisplay.h/.cpp — Complete Rewrite)
- **7-layer draw node architecture** ordered by z-depth: background, noise, sweep, trails, blips, overlay, labels
- **Sweep beam:** Filled triangle wedge with 30° phosphor glow trail (20 segments), non-linear brightness decay (`pow(fadeRatio, 0.6f)`)
- **Radar blips:** Color-coded by IFF, sized by RCS, glow effect for bright blips
- **Track history trails:** 8 trail points, 1.5s intervals, 15s max age, fading opacity
- **Selection highlight:** Animated pulsing corner brackets with ring
- **Range rings:** Labeled in nautical miles for authentic military display
- **Radar noise:** 120 dots with quadratic radial distribution (ground clutter effect)
- **Missile flight trails:** Dashed cyan lines from battery to target
- **Battery position labels:** P1-P3 (Patriot), H1-H3 (Hawk) with type-coded icons
- **Territory zone:** Dashed circle at defense zone perimeter
- **findNearestTrack():** Centralized hit detection with proper coordinate conversion

#### Game HUD Enhancements (GameHUD.cpp)
- AN/TSQ-73 CONSOLE header
- Controls help panel (1-3 PATRIOT, 4-6 HAWK, F=FIRE, A=ABORT)
- Range band indicator (CRITICAL/SHORT/MEDIUM/LONG)
- Available batteries list for selected track
- Battery panel `>` marker for available batteries
- Reload countdown timer and engaged target display (`->TK-003`)
- Negative score turns red
- Range displayed in both NM and km

#### Fire Control System (FireControlSystem.cpp)
- Battery positions scaled: Patriot 5→15 km, Hawk 3→8 km from center
- Proportional to new radar range for realistic coverage layout

#### Aircraft Generator (AircraftGenerator.cpp)
- Spawn range now dynamic: `randomRange(maxRange * 0.85f, maxRange)` — 393-463 km

#### Radar Scene Bug Fix (RadarScene.cpp)
- Fixed critical touch-select bug: was using `getSweepAngle()` instead of radius for pixel range calculation
- Delegated to `RadarDisplay::findNearestTrack()` with proper `convertToNodeSpace()`
- Added click-empty-space deselection
- Stub console output updated to show NM instead of km

#### Test Suite Updates (212 tests — all passing)
- `test_game_types.cpp`: Updated Patriot (160km), Hawk (45km), territory (25km), radar (463km); added `test_radar_range_nm` and `test_nm_km_conversion`
- `test_missile_battery.cpp`: Out-of-range test updated (80km → 170km for Patriot 160km range)
- `test_aircraft_generator.cpp`: Spawn range assertions updated (85km → 393km, 100km → 463km)
- `test_cocos2dx_validation.cpp`: Stub mode radar constant updated (100km → 463km)

---

*This log will be updated with each subsequent commit.*
