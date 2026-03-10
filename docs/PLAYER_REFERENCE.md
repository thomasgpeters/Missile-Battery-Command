# Player Quick Reference

## HQ Status Indicator

The overhead HUD shows current HQ status:

| Display | Meaning |
|---------|---------|
| `HQ:OPERATIONAL RDR:ON` | Full capability — surveillance radar scanning, voice/data links active to all batteries |
| `HQ:RELOCATING RDR:OFF` | HQ is displacing — all systems offline, batteries operating autonomously |
| `HQ:SETTING UP RDR:ON` | HQ at new position — radar scanning but comms not yet restored |
| `HQ:SETTING UP RDR:OFF` | HQ at new position — still bringing systems online |
| `HQ:DEGRADED` | Partial capability — some systems impaired |

---

## What "HQ Offline" Means to You

When HQ displaces, **you** go dark. The AN/TSQ-73 console you're sitting at
is being packed up and moved.

### What You Lose
- Your radar display goes blank (AN/TPS-43E surveillance radar is offline)
- You cannot issue engagement commands to batteries
- You cannot coordinate fire distribution
- You have no IFF picture
- Voice and data links to all batteries are down

### What Still Works
- **Every battery keeps fighting** — Patriot, Hawk, and Javelin all have
  their own organic tracking radars and can acquire and engage targets
  independently
- Batteries that were READY before HQ went offline remain READY
- Batteries that were ENGAGED continue their engagements to completion
- Batteries with targets in their envelope will engage autonomously

### The Risk of Autonomous Mode
- No coordinated fire distribution — two batteries may engage the same target
- No long-range early warning — targets detected later (at organic radar
  range, not 250 NM surveillance range)
- No centralized IFF — each battery making its own friend-or-foe call
- Possible wasted missiles on the same track
- Possible gaps in coverage that nobody is assigned to watch

---

## Battery Status Quick Reference

| Status | Meaning | Can Engage? |
|--------|---------|-------------|
| READY | Operational, missiles loaded | Yes |
| TRACKING | Acquired target, preparing | Yes (if redirected) |
| ENGAGED | Missile in flight | No (wait for result) |
| RELOADING | Loading missiles from stock | No (wait for reload) |
| OFFLINE | Displacing or out of ammo | No |
| DESTROYED | Battery destroyed | No |

---

## Displacement Timing

| Unit | Displacement Time | Notes |
|------|------------------|-------|
| HQ (Missile Minder) | ~2 hours | 45 min teardown + 30 min travel + 45 min setup |
| Patriot Battery | ~60 minutes | AN/MPQ-53 phased array takes time to calibrate |
| Hawk Battery | ~45 minutes | Multiple radar systems to erect and align |
| Javelin Team | ~15 minutes | Foot-mobile — grab CLU and missiles, walk |

**Game time compression:** 1 game-second = 1 real-minute during displacement.

---

## Displacement Rules of Thumb

1. **Never displace everything at once** — Stagger your moves
2. **Displace during lulls** — Between enemy waves, not during them
3. **Keep at least 2 of 3 batteries online** per type — If HAWK-1 is
   displacing, HAWK-2 and HAWK-3 should be covering
4. **HQ displacement is the biggest decision** — You lose all coordination
   for ~2 hours
5. **Javelin teams are cheap to move** — They're fast, use them to plug gaps
6. **Patriot is expensive to move** — 60 minutes offline for your long-range
   umbrella

---

## Communications Architecture

Your fire batteries are spread across miles. They connect back to HQ via:

- **Point-to-point microwave** — Directional antennas providing voice and
  data between HQ and each battery site. These are line-of-sight and must
  be aimed at the receiving station.
- **SATCOM** — Satellite voice and data links, used as backup or for
  batteries at extended range / behind terrain.
- **Data circuits** — Digital track data, engagement commands, and battery
  status flowing through the L-112 processors.
- **Voice circuits** — Tactical voice coordination between HQ operators and
  battery fire direction centers.

When HQ displaces, all point-to-point antennas come down and all circuits
go dark. When HQ sets back up, the microwave antennas are re-erected and
aimed at each battery site to restore connectivity.

When a **battery** displaces, only that battery's link goes down. All other
batteries maintain their connection to HQ (if HQ is operational).

---

## Engagement Priority Guide

When HQ is online and you have the full picture, assign targets wisely:

| Threat | Best Battery | Why |
|--------|-------------|-----|
| High-altitude bomber | Patriot | Best altitude envelope (up to 80,000 ft) |
| Fast fighter at medium alt | Patriot or Hawk | Patriot for range, Hawk if closer |
| Low-altitude attacker | Hawk | Hawk goes down to 100 ft; Patriot struggles below 1,000 ft |
| Slow drone | Hawk or Javelin | Don't waste a Patriot on a drone |
| Stealth fighter | Javelin (if close) | IR seeker ignores stealth — heat is heat |
| Unknown/pending IFF | **Wait** | Confirm before engaging — could be friendly |

---

## Weapon System Envelopes

```
Altitude (ft)
80,000 ┤ ←──── PATRIOT ────→
       │       (3-160 km)
       │
45,000 ┤ ←── HAWK ──→
       │   (1-45 km)
       │
15,000 ┤ ← JAVELIN →
       │  (0.5-55 km)
 1,000 ┤─── PATRIOT min alt ───
   100 ┤─── HAWK min alt ───
     0 ┼──────────────────────────→ Range (km)
       0    25    50   100   160
```

Note the gap: below 1,000 ft, Patriot cannot engage. That low-altitude
corridor is Hawk's domain. If all Hawks are offline (displacing or out of
missiles), that corridor is open to the enemy.
