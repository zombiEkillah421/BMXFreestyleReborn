# BMX Freestyle Reborn — UE5 playable prototype source v0.1.0

**Project type:** Standalone Windows PC game prototype (Unreal Engine C++)  
**Base engine:** Unreal Engine **5.6**; intended to migrate to newer UE5 versions through Unreal's project converter.  
**Dependencies:** Unreal's built-in **ProceduralMeshComponent** plugin; no marketplace purchases, imported assets, external libraries, or existing maps.  
**Build/test status:** Source and structure inspected with automated static checks. **Not compiled or playtested in Unreal Engine** because Unreal Editor/UnrealBuildTool is not available in the development environment. This archive is **not a precompiled Windows .exe**.

## What's included

- Arcade-assisted BMX pawn: pedal, brake, reverse, steer, bunny-hop, gravity, landing detection, stability-oriented resets and bail recovery.
- Separate follow/orbit camera with independent mouse/right-stick control and auto-follow after camera input goes idle.
- Six trick inputs: barspin, tailwhip, 360 left, 360 right, frontflip, backflip.
- Sequential air-trick queue, incomplete-trick bails, banking combo points on landing, 0.35 incremental combo multiplier, session total score.
- Training park built entirely in C++ at runtime: 60x60m pad, four curved quarter-pipes, two bank ramps, tabletop, ledge, rails, perimeter bounds, sunlight.
- Visual prototype bike and rider built from stock Unreal primitive meshes: **placeholders**, not finished character animation or physics simulation.
- HUD with speed, points, active combo and controls. Keyboard and standard Xbox-style gamepad mappings.

## How to open (Windows)

1. Install **Unreal Engine 5.6** via Epic Games Launcher. Unreal 5.7+ may prompt to convert the project; use a *copy* when converting.
2. For this **C++ project**, have **Visual Studio 2022** with the *Game development with C++* workload and Unreal-required Windows SDK/MSVC components installed.
3. Extract the ZIP to a folder with a short path, e.g. `C:\Games\BMXFreestyleReborn\`.
4. Double-click `BMXFreestyleReborn.uproject`. Let Unreal generate/compile the C++ modules if prompted. `OPEN_PROJECT.bat` is also provided to open the project using the file association.
5. If Unreal opens an empty editor world, click **Play**. The game mode creates the training park at runtime; no park Blueprint/map content is needed for this prototype.
6. For a standalone package, open **Project Settings -> Maps & Modes**, save an empty game level into `Content/Maps/BMXTraining.umap`, and set both **Game Default Map** and **Editor Startup Map** to it. Package for **Windows** through **Platforms -> Windows -> Package Project**. The generated park spawns into that empty level automatically.

**Important:** The project initially points to the built-in `/Engine/Maps/Entry` map so the source can be opened without a generated `.umap`. You should save the proper empty game map as step 6 before packaging. This is a source-first prototype, not a packaged retail build.

## Keyboard controls

| Control | Key |
| --- | --- |
| Pedal / brake-reverse | W / S |
| Steer left/right | A / D |
| Orbit camera | Move mouse |
| Bunny hop | Space |
| Barspin | B |
| Tailwhip | T |
| Spin 360 left/right | Q / E |
| Frontflip/backflip | F / G |
| Recenter camera | C |
| Reset to training start | R |

## Xbox-style controller controls

| Control | Button |
| --- | --- |
| Pedal / brake / steer | Left stick |
| Orbit camera | Right stick |
| Bunny hop | A |
| Barspin | X |
| Tailwhip | Y |
| 360 left / right | LB / RB |
| Backflip / frontflip | D-pad down / up |
| Reset | B |
| Recenter camera | Right stick click |

## What to test first

- Starting view: visible bike, practice park, speed and score HUD.
- Hold W, turn A/D, release W: acceleration, steering, coasting and deceleration.
- Jump on flat ground, land without a trick: no bail, no phantom scoring.
- Jump then press B: completes barspin; landing banks +150 points.
- Jump then press B and T: both animate sequentially; landing banks multiplied score if sufficient airtime.
- Start a backflip too close to landing: bail and lose only the unbanked combo; reset automatically.
- Ride onto curved ramps. Verify no sudden launch, fall-through, or wall collision.
- Move mouse independently: camera orbits bike without changing direction. C recenters view.
- Press R: return to starting point, retaining **total** score.
- Play with a connected Xbox-style gamepad.

For a full in-engine QA matrix, see `Docs/PLAYTEST_CHECKLIST.md`.

## Gameplay caveats (prototype v0.1.0)

- This is **not** a fully simulated bicycle; arcade velocity and raycast ground-following are used intentionally to establish a stable foundation.
- The visual bike and rider are static primitive shapes; trick rotations are component-based, not skeletal animation.
- Rails are scenery/collision now; there is **no grind-scoring system yet**.
- No menus, audio, sounds, saves, real career mode, multiplayer, or custom art.
- No independent UE runtime testing has been performed here: real-world playtesting may reveal compiler/API or physics issues that need iteration.
- Material colors are default Unreal primitives; custom visual theme is a later milestone.

## Code organization

- `BMXGameMode` spawns the world, default rider pawn, and score HUD.
- `BMXTrainingPark` creates the park, collision-enabled procedural quarter-pipes, wedges and obstacles.
- `BMXRiderPawn` handles the bike, camera, inputs, tricks, landings, combos and recovery.
- `BMXHUD` renders in-game score and control hints without requiring UMG/Blueprint assets.
- `Config/DefaultInput.ini` defines keyboard and gamepad bindings using Unreal's legacy input path for zero-asset bootstrap. Later we'll migrate to Enhanced Input mappings for player rebinds.

## Legal and originality

This is original prototype code informed by the **genre and feel** of 2001-era BMX arcade games. It **does not include** assets, levels, audio, likenesses, trademarks, logos, or code from *Dave Mirra Freestyle BMX 2*. A commercial/public release should use its own final branding and assets or obtain suitable permissions.
