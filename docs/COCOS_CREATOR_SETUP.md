# Cocos Creator 3.8 — Project Setup Guide

How to open the project, build the dual-console scene, and preview the
AN/TSQ-73 Missile Minder with fire controls and HUD.

---

## 1. Install Cocos Creator 3.8

Download from: https://www.cocos.com/en/creator-download

- Pick **Cocos Creator 3.6.x or later** (the project targets 3.6+).
- Install normally — Windows, macOS, or Linux AppImage all work.
- On first launch it will ask you to sign in / create an account (free).

## 2. Open the Project

1. Launch **Cocos Creator**.
2. **Open Other Project** → navigate to:
   ```
   Missile-Battery-Command/cocos-creator-project/
   ```
3. Click **Open**. The editor will import assets and compile TypeScript.
   First import may take 30–60 seconds.

## 3. Create the Dual-Console Scene

The components are all coded — you just need to wire them into a scene.

### 3a. New Scene

1. In the **Assets** panel, right-click `assets/scenes/` →
   **Create → Scene**.
2. Name it `DualConsole.scene`.
3. Double-click to open it. You'll see a default Camera.

### 3b. Canvas Setup

1. In the **Hierarchy**, right-click the root → **Create → UI → Canvas**.
2. Select the Canvas node.  In **Inspector**:
   - Design Resolution: **1920 × 1080**
   - Fit Height: **checked**
   - Fit Width: **unchecked**

### 3c. Shelter Background

1. Right-click Canvas → **Create → 2D → Graphics**.
2. Rename to `ShelterBG`.
3. Add component: **UITransform** (if not already present).
   Set Content Size to **1920 × 1080**.

### 3d. Left Console

**IMPORTANT: node order matters — radar FIRST (behind), frame SECOND (on top
as a bezel mask with transparent center).**

1. Right-click Canvas → **Create → Empty Node**. Rename: `LeftConsole`.
2. Set Position: **X = −350, Y = −40**.
3. Add component **UITransform**, Content Size **720 × 720**.
4. Right-click `LeftConsole` → **Create → 2D → Graphics**.
   Rename: `LeftRadar`. Add component: **RadarDisplay**.
5. Right-click `LeftConsole` → **Create → 2D → Graphics**.
   Rename: `LeftFrame`. Add component: **ConsoleFrame**.
   In Inspector, **uncheck** `Draw Background` (the shelter BG is drawn
   by the ShelterBG node, not each individual console).
   Ensure `Scope Radius` matches the RadarDisplay `Radius` (both 280).

### 3e. Right Console

1. Right-click Canvas → **Create → Empty Node**. Rename: `RightConsole`.
2. Set Position: **X = +350, Y = −40**.
3. Add component **UITransform**, Content Size **720 × 720**.
4. Same as left — create `RightRadar` (+ RadarDisplay) **first**,
   then `RightFrame` (+ ConsoleFrame, uncheck `Draw Background`)
   **second**. The frame draws on top as a bezel with a transparent
   circular cutout so the radar sweep shows through.

### 3f. Overhead HUD Panel

1. Right-click Canvas → **Create → 2D → Graphics**.
   Rename: `OverheadHUD`.
2. Set Position: **X = 0, Y = 380**.
3. Add component: **OverheadHUD**.
4. In Inspector, set:
   - Display Width: **760**
   - Display Height: **200**

### 3g. Wire Up DualConsoleScene

1. Select the **Canvas** node.
2. **Add Component → DualConsoleScene**.
3. In Inspector, drag-and-drop:

   | Property              | Drag This Node        |
   |-----------------------|-----------------------|
   | Left Radar Display    | `LeftRadar`           |
   | Left Console Frame    | `LeftFrame`           |
   | Right Radar Display   | `RightRadar`          |
   | Right Console Frame   | `RightFrame`          |
   | Overhead HUD          | `OverheadHUD`         |
   | Shelter Graphics      | `ShelterBG`           |

### Final Hierarchy

```
Canvas  [DualConsoleScene]
├── ShelterBG          [Graphics]           ← draws first (behind everything)
├── LeftConsole
│   ├── LeftRadar      [Graphics, RadarDisplay]  ← radar behind
│   └── LeftFrame      [Graphics, ConsoleFrame, drawBackground=OFF]  ← bezel on top
├── RightConsole
│   ├── RightRadar     [Graphics, RadarDisplay]  ← radar behind
│   └── RightFrame     [Graphics, ConsoleFrame, drawBackground=OFF]  ← bezel on top
└── OverheadHUD        [Graphics, OverheadHUD]   ← draws last (on top)
```

**Node order = draw order.** Earlier siblings render behind later ones.
The frame draws ON TOP of the radar as a bezel mask with a transparent
circular cutout in the center, so the sweep shows through the hole.

## 4. Preview

1. **Ctrl+P** (or click the Play ▶ button in the toolbar).
2. The browser will open with the dual-console view:
   - Two PPI radar scopes with sweep beam and blips
   - Console bezels with illuminated pushbutton panels
   - Red LED overhead displays showing track table and battery status
   - Shelter interior background with dim red lighting

## 5. Controls

| Key       | Action                                      |
|-----------|---------------------------------------------|
| Click     | Select a track on the left scope            |
| 1 / 2 / 3| Assign PATRIOT-1 / 2 / 3 to selected track |
| 4 / 5 / 6| Assign HAWK-1 / 2 / 3                      |
| 7 / 8 / 9| Assign JAVELIN-1 / 2 / 3                   |
| F         | Fire (authorize engagement)                 |
| A         | Abort engagement                            |

## 6. Single-Console Scene (Alternative)

If you prefer the single-operator view, repeat steps 3a–3g but use
**SingleConsoleScene** instead of DualConsoleScene. You only need one
radar display and one console frame. Design resolution **1280 × 960**.

## 7. Build for Web

To create a deployable web build:

1. **Project → Build** (or Ctrl+Shift+B).
2. Platform: **Web Desktop** or **Web Mobile**.
3. Start Scene: `DualConsole`.
4. Click **Build**, then **Run**.
5. Output goes to `cocos-creator-project/build/web-desktop/`.
   Open `index.html` in any browser.

---

## Troubleshooting

**"Cannot find module" errors on import?**
The project uses barrel exports from `assets/scripts/game/index.ts`.
Make sure all `.ts` files are present under `assets/scripts/`.

**Graphics component not rendering?**
Each Graphics node needs a **UITransform** with a non-zero Content Size.
Cocos Creator 3.8 requires this for 2D rendering bounds.

**Scene is black?**
Check the Camera node is set to **UI** mode (Projection: Orthographic)
and the Canvas is in the camera's visibility layers.

**Radar sweep hidden behind the console frame?**
The radar node must come **before** the frame in the Hierarchy (earlier =
behind). The frame draws on top as a bezel with a transparent circular
cutout so the sweep shows through. Ensure `Scope Radius` on ConsoleFrame
matches `Radius` on RadarDisplay (both default to 280). Also uncheck
`Draw Background` on ConsoleFrame in dual-console mode.

**Radar scope is tiny / off-center?**
Verify the RadarDisplay node has UITransform Content Size of at least
**560 × 560** (2 × scopeRadius). Anchor should be (0.5, 0.5).
