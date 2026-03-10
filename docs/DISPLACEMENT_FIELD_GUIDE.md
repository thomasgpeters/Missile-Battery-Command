# Displacement Field Guide

## Operational Displacement — What It Really Means

"Displacement" is the military term for tearing down a tactical position,
moving to a new location, and setting back up. In air defense, displacement is
a survival tool — if the enemy discovers your position through signals
intelligence, reconnaissance, or a strike that got through, you need to move
before they can target you.

But displacement has a cost: **every unit that displaces goes offline for the
duration.** The art of air defense displacement is ensuring you never leave
your airspace undefended.

---

## The Cardinal Rule

> **The battalion never displaces as a whole. Each element displaces
> independently, on its own schedule, and never all at the same time.**

This means:
- HQ displaces separately from the fire units
- Each Hawk battery displaces on its own timeline
- Each Patriot battery displaces on its own timeline
- Javelin teams displace independently (and quickly — they're on foot)
- At least one battery of each type should remain operational while another
  displaces

The commander (you) must stagger displacements so the defense zone always has
coverage.

---

## HQ (HD) Displacement

When HQ displaces, you are tearing down the AN/TSQ-73 Missile Minder and all
its supporting equipment. This is the most complex displacement in the
battalion.

### What Gets Torn Down

1. **AN/GSS-1 Radar Antenna** — The surveillance radar antenna sits on a
   separate vehicle. The mast must be lowered, the antenna folded and secured,
   all RF cables disconnected and coiled.

2. **AN/TSQ-73 Shelter (S-280)** — The console itself is bolted to racks
   inside the shelter. The dual L-112 processors must be powered down in
   sequence (not just killed — a proper shutdown to preserve memory state).
   All data cables running out the back of the shelter to the antenna truck,
   the generator, and the comms equipment are disconnected.

3. **Generator (MEP-006A)** — 440V 3-phase power distribution is disconnected
   from all shelters and equipment. The generator is shut down, fuel lines
   secured, and the unit prepped for tow.

4. **Communications Systems** — The site-to-site voice and data links to all
   fire batteries are terminated:
   - Point-to-point microwave antennas are taken down and stowed
   - SATCOM terminals are disconnected and secured
   - Voice and data circuits to all fire batteries go dark
   - This is the moment the fire batteries lose centralized coordination

5. **Cable Runs** — Hundreds of meters of data cable and power cable connecting
   all the vehicles within the HQ battery area (antenna truck to shelter,
   shelter to generator, shelter to comms equipment). These are coiled onto
   reels and loaded.

### HD Displacement Timeline

| Phase | Duration | What Happens |
|-------|----------|-------------|
| **Teardown** | ~45 minutes | Power down processors, lower antenna, disconnect cables, secure equipment |
| **Convoy Travel** | ~30 minutes (variable) | HD vehicles move to new position via planned route |
| **Setup** | ~45 minutes | Erect antenna, lay cables, start generator, boot processors, calibrate displays |
| **Total** | **~2 hours** | Full displacement cycle |

### Battalion Geography

An ADA Hawk battalion is **spread out across miles.** The fire batteries are
not co-located with HQ — they are deployed at their tactical positions across
the defense zone, connected back to the AN/TSQ-73 by dedicated communications
systems:

- **Point-to-point microwave antennas** — Site-to-site voice and data links
  between HQ and each fire battery. These are directional antennas that must
  be aimed at the receiving site.
- **Satellite communications (SATCOM)** — Backup voice and data path,
  especially for batteries at extended range or in terrain that blocks
  line-of-sight microwave.
- **Voice circuits** — Tactical voice coordination between HQ and battery
  command posts (fire direction centers).
- **Data circuits** — Digital track data, engagement commands, and status
  reporting flowing between the Missile Minder and each battery's fire
  control equipment.

When HQ displaces, **all of these links go down** — the point-to-point
antennas are taken down with the rest of the equipment, and the data
circuits through the L-112 processors stop.

### What the Batteries Experience

When HD goes dark:

- **Long-range surveillance stops** — The AN/TPS-43E radar (250 NM range) goes
  offline. No one is painting the long-range picture anymore.

- **Fire distribution stops** — The Missile Minder console is what tells
  Battery A to take Track 101 while Battery B takes Track 102. Without HQ,
  there is no one coordinating who shoots at what.

- **Voice and data links go down** — The point-to-point microwave and SATCOM
  links that carried track data and engagement commands from HQ to the fire
  batteries are disconnected during teardown. Batteries lose the data feed.

- **Batteries switch to autonomous operations** — Each battery falls back on
  its own organic radar. They are fully capable weapon systems on their own:
  - Patriot: AN/MPQ-53 phased array (still very capable, 160 km range)
  - Hawk: AN/MPQ-46 HPI (45 km range, excellent for its role)
  - Javelin: CLU IR/FLIR (visual/infrared, short range, no radar)

- **No centralized IFF** — Each battery must perform its own identification.
  The HQ's coordinated IFF picture is gone.

### What Autonomous Mode Means for the Player

When HQ is offline, your fire batteries **can still engage.** They are not
helpless — each one has its own tracking radar and engagement capability. What
they lose is:

1. **Early warning** — Without the TPS-43E, targets are detected later
   (closer) because organic radars have shorter range than the surveillance
   radar.

2. **Coordinated fire distribution** — Two batteries might independently
   decide to engage the same target, wasting missiles. Or a gap might open
   that neither battery covers because there is no one assigning sectors.

3. **IFF coordination** — Each battery has to make its own friend-or-foe
   determination without the battalion-level picture.

In game terms: the batteries keep fighting, but the player loses the ability
to direct their fire from the console. Engagements during HQ displacement
happen automatically (or not at all) until HQ comes back online.

---

## Battery Displacement

Individual fire batteries displace independently from HQ. The procedures are
different for each system.

### Patriot Battery Displacement (~60 minutes)

The Patriot is the heaviest system to move. The AN/MPQ-53 phased array radar
is a large, vehicle-mounted system with significant setup requirements.

**Teardown:**
- Radar antenna stowed to travel position
- Missile canisters secured on the launcher
- Fire control equipment powered down
- Cable connections broken between the radar vehicle, ECS (Engagement Control
  Station), and launcher

**Setup at new position:**
- Radar leveled and calibrated
- Launcher aligned
- Fire control equipment booted and initialized
- Radar warming up and performing self-test
- Data link to HQ re-established (if HQ is operational)

### Hawk Battery Displacement (~45 minutes)

The Hawk system is somewhat lighter than Patriot but still involves multiple
vehicles.

**Teardown:**
- AN/MPQ-46 HPI (High Power Illuminator) lowered and secured
- CW illuminator antenna stowed
- Launcher rail assemblies secured
- Missile rounds checked and secured for transport

**Setup at new position:**
- Radar erected and leveled
- CW illuminator aligned
- Launchers emplaced and oriented
- System checks and calibration
- Data link to HQ re-established (if HQ is operational)

### Javelin Team Displacement (~15 minutes)

Javelin teams are foot-mobile. "Displacement" means the team picks up their
CLUs and missile tubes and walks to a new position.

**What they carry:**
- Command Launch Unit (CLU) with IR/FLIR seeker
- Missile rounds (BCUs inserted just before firing)
- Radio for communication with HQ (when HQ is up)
- Personal equipment

Javelin is the fastest to displace and the least disruptive to the defense.
A team can move to a new position in 15 minutes and be ready to engage
immediately on arrival.

---

## Staggered Displacement — The Commander's Dilemma

The tactical challenge is maintaining continuous coverage while moving your
pieces around the board. Consider this scenario:

**Bad plan:**
> HAWK-1, HAWK-2, and HAWK-3 all displace simultaneously.
>
> Result: For 45 minutes, the mid-range defense layer is completely gone.
> Any target between 1–45 km altitude below 45,000 ft has a free lane
> to the territory. Patriot can cover some of this, but not the low-altitude
> threats that Hawk excels at.

**Good plan:**
> HAWK-1 displaces first. HAWK-2 and HAWK-3 remain operational, covering
> HAWK-1's sector between them.
>
> When HAWK-1 is set up at the new position and reports READY, HAWK-2
> begins displacement.
>
> HAWK-3 only displaces after HAWK-2 is back online.
>
> Result: At no point are more than one-third of your Hawk batteries offline.
> Defense is degraded but continuous.

The same principle applies to Patriot, to HQ, and to any combination. The
commander must decide:
- **When** to displace (between waves? during a lull?)
- **Who** displaces first (the most exposed unit? the one closest to the
  new position?)
- **What risk is acceptable** (can I displace two Hawks at once if the
  threat is primarily high-altitude?)

---

## HQ Displacement During Active Engagement

This is the hardest decision in the game. The enemy has identified your HQ
position. You need to move. But there are aircraft inbound and your batteries
need coordination.

Options:
1. **Hold position** — Keep HQ online, accept the risk of being targeted.
   Your batteries get full coordination but HQ is vulnerable.

2. **Displace immediately** — Go dark for ~2 hours. Batteries fight
   autonomously. You lose the ability to direct fires, but HQ survives to
   fight from a new position.

3. **Displace during a lull** — Wait for the current wave to be engaged,
   then begin teardown before the next wave arrives. This requires good
   timing and awareness of the threat cycle.

There is no right answer — it depends on the tactical situation, your
remaining missile stocks, and how confident you are that your batteries can
handle autonomous operations without wasting missiles on the wrong targets.

---

## Setup Phase — Coming Back Online

When HQ or a battery arrives at a new position, it doesn't instantly become
operational. The setup phase is the reverse of teardown:

### HQ Setup Sequence
1. Generator started, power distribution connected
2. Cables laid between vehicles within the HQ area (antenna truck, shelter,
   generator, comms equipment)
3. AN/GSS-1 antenna erected and aimed
4. L-112 processors booted (takes time — these are 1970s-era computers)
5. Console displays calibrated and initialized
6. Surveillance radar begins scanning — **radar comes online** (50% through
   setup)
7. Point-to-point microwave antennas erected and aimed at each fire battery
   site; SATCOM terminals brought online
8. Voice and data circuits to fire batteries re-established — **comms come
   online**
9. Fire distribution resumes — **HQ is OPERATIONAL**

Note that radar comes back online before comms do. This means HQ can see
the air picture before it can talk to the batteries. There is a brief window
where the HQ operator can see what is happening but cannot direct fires —
the surveillance radar is scanning but the voice and data links to the fire
batteries haven't been re-established yet.

### Battery Setup Sequence
1. Vehicle positioned and leveled at new site
2. Radar erected (Patriot/Hawk) or team deployed (Javelin)
3. System self-test and calibration
4. Status transitions from OFFLINE to READY
5. Point-to-point comms link aimed at HQ site and re-established (if HQ is
   operational at its position)
