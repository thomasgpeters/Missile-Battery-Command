# Missile Battery Command — Game Scenario

## Operational Setting

You are the Air Defense Artillery (ADA) Officer commanding a composite air
defense battalion defending a critical asset area. Your battalion operates from
the **AN/TSQ-73 Missile Minder** — a mobile fire distribution center housed in
an S-280 shelter mounted on a 5-ton truck.

From your two PPI radar scopes inside the shelter, you coordinate the fires of
nine weapon systems arranged in layered defense around your territory.

## The Battalion

### Headquarters Battery (HD)

The battalion headquarters is you — the AN/TSQ-73 and its supporting equipment:

| Equipment | Purpose |
|-----------|---------|
| AN/TSQ-73 Missile Minder | Fire distribution console (2 operator scopes) |
| AN/TPS-43E | Long-range surveillance radar (250 NM / 463 km range) |
| AN/GSS-1 | Radar antenna assembly (erected on separate truck) |
| Dual L-112 Processors | Track data processing and fire distribution computing |
| S-280 Shelter | Climate-controlled van housing the console |
| MEP-006A Generator | 60kW tactical generator providing 440V 3-phase power |
| M577 Command Vehicle | Command post vehicle for coordination |

**HD Personnel:** 35 soldiers operating in shifts

### Fire Units

Your battalion controls three types of air defense weapon systems, each with
its own organic tracking radar. They do **not** depend on HQ to engage — they
can acquire and fire autonomously using their own radars. What HQ provides is
the big picture: long-range early warning, centralized fire distribution, and
coordinated engagement sequencing so batteries don't waste missiles on the same
target.

#### Patriot Batteries (MPMB) — MIM-104

Three Patriot batteries positioned at ~15 km in a triangle formation around the
defense zone. These are the long-range, high-altitude backbone of the defense.

| Spec | Value |
|------|-------|
| Designation | PATRIOT-1, PATRIOT-2, PATRIOT-3 |
| Tracking Radar | AN/MPQ-53 phased array |
| Max Range | 160 km |
| Min Range | 3 km |
| Altitude Envelope | 1,000 – 80,000 ft |
| Ready Missiles | 4 per launcher |
| Missile Speed | 1,700 m/s |
| Kill Probability | ~85% (baseline) |
| Displacement Time | ~60 minutes |

#### Hawk Batteries (HSAMB) — MIM-23

Three Hawk batteries at ~8 km, filling the gap between Patriot's minimum
altitude and the inner defense zone. Optimized for medium-range, low-to-medium
altitude threats.

| Spec | Value |
|------|-------|
| Designation | HAWK-1, HAWK-2, HAWK-3 |
| Tracking Radar | AN/MPQ-46 High-Power Illuminator (HPI) |
| Max Range | 45 km |
| Min Range | 1 km |
| Altitude Envelope | 100 – 45,000 ft |
| Ready Missiles | 3 per launcher |
| Total Stock | 33 missiles per battery |
| Reload Crew | 3 loaders |
| Missile Speed | 900 m/s |
| Kill Probability | ~75% (baseline) |
| Displacement Time | ~45 minutes |

#### Javelin MANPADS Platoons — FGM-148

Three Javelin platoons at ~3 km — the last line of defense. Shoulder-launched,
infrared-guided, fire-and-forget. No radar signature, but limited range.

| Spec | Value |
|------|-------|
| Designation | JAVELIN-1, JAVELIN-2, JAVELIN-3 |
| Seeker | CLU IR/FLIR (infrared — no radar) |
| Max Range | 55 km (game value; operational range much shorter) |
| Min Range | 0.5 km |
| Altitude Envelope | Ground level – 15,000 ft |
| Ready Missiles | 2 |
| Missile Speed | 300 m/s |
| Kill Probability | ~65% (baseline) |
| Displacement Time | ~15 minutes (foot-mobile) |

**Key advantage:** IR guidance means stealth aircraft gain no benefit against
Javelin. The seeker tracks heat, not radar return.

### Communications

The battalion is **spread across miles** — fire batteries are not co-located
with HQ. All coordination happens over dedicated communications systems:

- **Point-to-point microwave antennas** — Directional site-to-site voice and
  data links between HQ and each fire battery
- **Satellite communications (SATCOM)** — Backup voice and data path for
  extended range or terrain-blocked links
- **Voice circuits** — Tactical voice coordination between HQ and battery
  fire direction centers
- **Data circuits** — Digital track data, engagement commands, and battery
  status reporting via the L-112 processors

When HQ displaces, these links go down — batteries switch to autonomous
operations using their own organic radars.

## Defense Zone Layout

```
                         N (0°)
                          |
                   PATRIOT-1 (15km)
                    /           \
                   /             \
          JAVELIN-1              HAWK-1
           (3km)                 (8km)
              |                    |
              |    [ MINDER-HQ ]   |
              |     (center)       |
              |                    |
          JAVELIN-3              HAWK-2
           (3km)                 (8km)
                   \             /
                    \           /
         PATRIOT-3    HAWK-3    PATRIOT-2
          (15km)     (8km)      (15km)
                          |
                   JAVELIN-2 (3km)
                          |
                         S (180°)
```

Territory radius: 25 km — anything hostile that penetrates this zone has
reached its target.

## The Threat

Enemy air forces will send increasingly sophisticated waves of aircraft against
your defended area:

| Threat | Characteristics |
|--------|----------------|
| Strategic Bomber | High altitude, fast, large radar cross-section |
| Fighter/Attack | Fast, agile, medium altitude |
| Tactical Bomber | Low-to-medium altitude, moderate speed |
| Recon Drone | Small, slow, hard to classify quickly |
| Attack Drone | Small, slow, may be carrying ordnance |
| Stealth Fighter | Reduced radar cross-section — harder to detect and engage |
| Civilian Airliner | **DO NOT ENGAGE** — catastrophic score penalty |
| Friendly Military | **DO NOT ENGAGE** — must verify IFF before firing |

### IFF (Identification Friend or Foe)

All tracks start as PENDING. Your IFF system (Mode 4 interrogation) takes time
to classify each track. The system has an error rate that increases with
difficulty level — at higher levels, the IFF may misidentify a hostile as
friendly or vice versa. You must cross-reference altitude, speed, heading, and
behavior to make engagement decisions. Shooting down a friendly or civilian
aircraft is the worst outcome in the game.

## Scoring

| Event | Points |
|-------|--------|
| Hostile destroyed | +100 (base) × type multiplier |
| First-shot kill | 2× multiplier |
| Hostile penetrates territory | -200 |
| Missile wasted (miss) | -25 |
| Friendly aircraft destroyed | -1,000 |

## Your Mission

Defend the territory. Manage your limited missile stocks. Make the right call
on IFF — sometimes you have to let a track close to confirm classification
before engaging. Coordinate your fire units so you don't burn three Patriots on
one bomber when a Hawk could handle it. And when the situation demands it,
displace your HQ or batteries to survive and keep fighting.
