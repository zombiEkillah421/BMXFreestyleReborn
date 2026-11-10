# BMX Freestyle Reborn (working title)

An original arcade BMX freestyle game for Windows PC, built in **Unreal Engine 5.8 (C++)**.
Inspired by the feel of 2001-era arcade BMX games: responsive riding, big airs, trick combos.

**Current version:** v0.2.0 — early playable prototype (see `Docs/CHANGELOG.md`).

## What's in the prototype

- **Training park** generated entirely in C++ at runtime: two big and two small quarter-pipes, a tabletop, a ledge, two rails and perimeter walls. No map or Blueprint assets are needed.
- **Arcade rider**: pedal, brake/reverse, steer, bunny hop. Two wheel-contact probes follow ramps.
- **Slope physics**: ramps slow you down going up and speed you up coming down. If you stall on a quarter-pipe, you roll back down backwards (fakie).
- **Vert airs**: hit a quarter-pipe with speed and you launch straight up, turn 180° in the air and drop back into the ramp.
- **Tricks**: barspin, tailwhip, 360 left/right, frontflip, backflip. Tricks queue into combos, each extra trick adds +0.35x to the multiplier, and repeated tricks in the same combo score less.
- **HUD**: score, combo, and a diagnostic readout for playtesting (toggle it with Tab).
- **Visuals**: placeholder bike and rider built from engine primitives. Final art comes later.

## Opening the project (Windows)

1. Install **Unreal Engine 5.8** and **Visual Studio 2022** with the *Game development with C++* workload.
2. Clone this repo to a short path, e.g. `C:\Games\BMXFreestyleReborn`.
3. Right-click `BMXFreestyleReborn.uproject` → **Generate Visual Studio project files** (the `.sln` isn't stored in Git).
4. Double-click `BMXFreestyleReborn.uproject` and let Unreal compile the C++ module.
5. Click **Play**. The game mode builds the park automatically on the empty Entry map.

**Getting updates:** `git pull`, then reopen the `.uproject` so it recompiles. If the HUD still shows the old version number, the rebuild didn't happen: close the editor and build from Visual Studio (Development Editor).

## Controls

| Action | Keyboard | Xbox-style gamepad |
| --- | --- | --- |
| Pedal / brake-reverse | W / S | Left stick |
| Steer | A / D | Left stick |
| Camera | Mouse | Right stick |
| Bunny hop | Space | A |
| Barspin / Tailwhip | B / T | X / Y |
| 360 left / right | Q / E | LB / RB |
| Frontflip / Backflip | F / G | D-pad up / down |
| Recenter camera | C | Right stick click |
| Reset to start | R | B |
| Toggle debug HUD | Tab | View / Back |

## Code layout

| File | Responsibility |
| --- | --- |
| `BMXGameMode` | Spawns park, sky, lighting and the rider |
| `BMXTrainingPark` | Builds the procedural ramps (visible mesh = collision) and boxes |
| `BMXRiderPawn` | Riding physics, wheel probes, launches/vert airs, tricks, combos, camera |
| `BMXHUD` | Canvas HUD; no UMG assets required |
| `Config/DefaultInput.ini` | Legacy input mappings (Enhanced Input migration planned) |

Riding and vert-air tuning values are exposed under **BMX|Tuning** and **BMX|Vert** on the rider pawn.

## Known limitations

- The bike isn't physically simulated. It uses arcade speed plus wheel-ray ground following.
- Tricks rotate primitive components. There's no skeletal animation yet.
- Rails are solid obstacles only. There's no grind system yet.
- No menus, audio, career mode or saving yet.

## Originality

All code and geometry here are original. The project doesn't include assets, levels, audio, likenesses, trademarks or code from any existing BMX game. The final release needs its own name and branding.
