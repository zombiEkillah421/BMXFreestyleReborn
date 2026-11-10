# BMX Freestyle Reborn (working title)

An original arcade BMX freestyle game for Windows PC, built in **Unreal Engine 5.8 (C++)**.
Inspired by the feel of 2001-era arcade BMX games: responsive riding, big airs, trick combos.

**Current version:** v0.3.5 — early playable prototype (see `Docs/CHANGELOG.md`).

## What's in the prototype

- **Training park** generated entirely in C++ at runtime: two big and two small quarter-pipes, a tabletop, a ledge, two rails, a **three-set dirt jump line** and perimeter walls. No map or Blueprint assets are needed.
- **Arcade rider**: pedal, brake/reverse, steer, bunny hop. Two wheel-contact probes follow ramps.
- **Slope physics**: ramps slow you down going up and speed you up coming down. If you stall on a quarter-pipe, you roll back down backwards (fakie).
- **Vert airs**: hit a quarter-pipe with speed and you launch straight up, turn 180° in the air and drop back into the ramp.
- **Free air rotation**: flip (F/G) and spin (Q/E) however far you like (backflips, frontflips, 180s, 360s, doubles) and combine them with any bike/body trick.
- **10 style tricks**: barspin, tailwhip, no-hander, superman, can-can, X-up, turndown, tabletop, nothing, tuck no-hander. Hold tricks score more the longer you hold them.
- **Manuals** link combos between landings, with a balance meter. Land a 180 and you ride away **fakie**.
- **Charged bunny hop**: hold Space to load up for a bigger pop.
- **Pumping**: hold W while dropping down a transition to gain speed, so back-and-forth airs on the quarter-pipes build higher.
- **Combos**: each extra trick adds +0.35x to the multiplier, and repeated tricks in the same combo score less.
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
| Pedal / brake-reverse (hold W downhill to pump) | W / S | Left stick |
| Steer | A / D | Left stick |
| Camera | Mouse | Right stick |
| Bunny hop (hold to charge) | Space | A |
| Manual (hold; W/S balance) | Left Shift | LT |
| **In the air:** flip / spin | F / G (front / back) · Q / E (left / right) | Left stick |
| Barspin / Tailwhip | B / T | X / Y |
| No-hander / Superman | J / K | LB / RB |
| Can-can / X-up | L / U | D-pad left / right |
| Turndown / Tabletop | I / O | D-pad down / up |
| Nothing / Tuck no-hander | H / N | RT / — |
| Reset / park start / dirt line | R / 1 / 2 | B / — / — |
| Recenter camera | C | Right stick click |
| Toggle debug HUD | Tab | View / Back |

On keyboard only F/G and Q/E rotate you, so pedaling or steering habits can't start a flip. On a gamepad the stick rotates you once it's returned to centre after takeoff.

## Code layout

| File | Responsibility |
| --- | --- |
| `BMXGameMode` | Spawns park, sky, lighting and the rider |
| `BMXTrainingPark` | Builds the procedural ramps (visible mesh = collision) and boxes |
| `BMXRiderPawn` | Riding physics, wheel probes, launches/vert airs, air rotation, style tricks, manuals, combos, camera |
| `BMXHUD` | Canvas HUD; no UMG assets required |
| `Config/DefaultInput.ini` | Legacy input mappings (Enhanced Input migration planned) |

Tuning values are exposed under **BMX|Tuning**, **BMX|Vert** and **BMX|Tricks** on the rider pawn.

## Known limitations

- The bike isn't physically simulated. It uses arcade speed plus wheel-ray ground following.
- Tricks rotate primitive components. There's no skeletal animation yet.
- Rails are solid obstacles only. There's no grind system yet.
- No grinds yet (next on the list), and no menus, audio, career mode or saving.

## Originality

All code and geometry here are original. The project doesn't include assets, levels, audio, likenesses, trademarks or code from any existing BMX game. The final release needs its own name and branding.
