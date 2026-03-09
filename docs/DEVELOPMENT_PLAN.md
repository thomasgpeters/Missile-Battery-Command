# Missile Battery Command - Development Plan
## AN/TSQ-73 Air Defense Artillery Battalion Simulator

### Game Concept
You are an Air Defense Artillery (ADA) operator at the controls of an AN/TSQ-73 missile
minder console. Your mission: protect your country's airspace from incoming hostile
aircraft — bombers, fighters, and drones — using a combination of Patriot and Hawk
surface-to-air missile batteries.

### Core Systems

#### 1. Radar Display (AN/TSQ-73 Console)
- Rotating radar sweep beam (PPI - Plan Position Indicator)
- Radar returns (blips) appear as the beam passes over aircraft
- Blips fade between sweeps (phosphor decay effect)
- Range rings at configurable intervals
- Azimuth markings (0-360 degrees)
- Radar clutter and noise for realism

#### 2. IFF System (Identification Friend or Foe)
- Automatic interrogation of radar contacts
- Returns: FRIENDLY (Mode 4 positive), HOSTILE (no response), UNKNOWN (pending)
- IFF interrogation takes time — contacts start as UNKNOWN
- False readings possible at higher difficulty levels

#### 3. Track Management
- Each identified contact gets a Track ID (e.g., TK-001)
- Track data: altitude (feet MSL), azimuth (degrees), speed (knots), heading
- Track classification: HOSTILE, FRIENDLY, UNKNOWN, PENDING
- Track history trail (dotted line showing past positions)

#### 4. Weapons Systems

##### Mobile Patriot Missile Batteries (MPMB) x3
- Long range (70+ km)
- High altitude capability (up to 80,000 ft)
- 4 missiles per launcher, reload time between salvos
- Best against bombers and high-altitude targets
- Designated: PATRIOT-1, PATRIOT-2, PATRIOT-3

##### Hawk Surface-to-Air Missile Batteries (HSAMB) x3
- Medium range (25-40 km)
- Medium-low altitude (treetop to 45,000 ft)
- 3 missiles per launcher, faster reload
- Best against fighters and low-altitude drones
- Designated: HAWK-1, HAWK-2, HAWK-3

#### 5. Fire Control
- Select target track → assign battery → authorize engagement
- Electronic firing commands transmitted to batteries
- Missile flight time based on distance
- Kill probability varies by weapon/target type
- Battle Damage Assessment (BDA) after engagement

#### 6. Aircraft Types (Threats)

| Type | Speed | Altitude | Radar Signature | Threat Level |
|------|-------|----------|-----------------|--------------|
| Strategic Bomber | 400-600 kts | 25,000-45,000 ft | Large | HIGH |
| Fighter/Attack | 500-900 kts | 5,000-35,000 ft | Medium | HIGH |
| Tactical Bomber | 350-500 kts | 15,000-30,000 ft | Medium-Large | MEDIUM |
| Recon Drone | 100-200 kts | 10,000-50,000 ft | Small | LOW |
| Attack Drone | 150-300 kts | 500-15,000 ft | Small | MEDIUM |
| Stealth Fighter | 500-800 kts | 15,000-40,000 ft | Very Small | CRITICAL |

#### 7. Friendly Aircraft
- Civilian airliners (must NOT engage)
- Friendly military patrols
- Identified via IFF Mode 4

### Game Levels / Difficulty

| Level | Max Concurrent | Spawn Rate | Stealth? | IFF Errors | Speed |
|-------|---------------|------------|----------|------------|-------|
| 1 | 3 | Slow | No | None | Slow |
| 2 | 5 | Medium | No | None | Normal |
| 3 | 8 | Medium | No | Rare | Normal |
| 4 | 10 | Fast | Yes | Occasional | Fast |
| 5 | 15 | Very Fast | Yes | Frequent | Fast |

### Scoring
- Hostile destroyed: +100 to +500 (based on threat level)
- Friendly destroyed: -1000 (game penalty)
- Hostile reaches territory: -200
- Missile wasted: -25
- Perfect engagement (first shot kill): 2x bonus

### Development Phases

#### Phase 1: Framework & Radar Display (Current)
- [x] Project structure and build system
- [x] AppDelegate and main scene
- [x] Radar PPI display with rotating beam
- [x] Range rings and azimuth markings
- [x] Basic radar return (blip) rendering

#### Phase 2: Aircraft & IFF
- [x] Aircraft entity with properties
- [x] Random aircraft generator (difficulty-scaled)
- [x] IFF interrogation system
- [x] Track ID assignment and management

#### Phase 3: Weapons & Fire Control
- [x] Missile battery entities
- [x] Fire control interface
- [x] Missile flight simulation
- [x] Kill probability calculation

#### Phase 4: Game Loop & UI
- [ ] HUD with track info panel
- [ ] Battery status displays
- [ ] Engagement authorization flow
- [ ] Score tracking
- [ ] Game over conditions

#### Phase 5: Polish
- [ ] Sound effects (radar sweep, missile launch, explosions)
- [ ] Visual effects (explosions, missile trails)
- [ ] Radar clutter/noise
- [ ] Menu system and settings
- [ ] Difficulty progression
