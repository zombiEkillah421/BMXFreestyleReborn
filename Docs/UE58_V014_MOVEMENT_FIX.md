# BMX Freestyle Reborn v0.1.4 — UE 5.8 movement-stability patch

## Issue reported
The bike initially refuses to move until Space is pressed, then can become blocked/suspended near a ramp or obstacle until R resets it. The supplied 27-second Editor recording shows intermittent movement and the speed HUD; an actual coordinate/collision log was not available.

## Code changes
- Fix spawn height: place the collision capsule 5 cm above the starting floor rather than exactly intersecting/touching it.
- Ground probe starts 68 cm above the bike's feet instead of 425 cm above the pawn center. This avoids treating a high overhead ramp as the current riding surface.
- Restrict grounded floor snapping to manageable rise/drop distances; validate real post-sweep height before entering grounded state.
- On-ground traversal: lift over the sampled surface, sweep horizontal movement, then settle vertically instead of combining into a single potentially blocking sweep.
- In air: sweep horizontal translation independently from gravity/vertical movement. A collision on a ramp wall therefore cannot cancel descent.
- Slow displayed speed after a blocking collision; allow a small tangent slide along obstacles.
- Add failsafe recovery if gravity is blocked by mesh overlap for more than 0.8 seconds, or airborne state persists excessively.
- Add HUD input/physics readout: `INPUT W/S +1.0 | GROUND`, `AIR`, or `BLOCKED`.
- Preserve previous camera, tricks, score, bike visuals, park, UE 5.8 targets and input mappings.

## Install over a working v0.1.3 project
1. Completely exit Play in Editor and close Unreal Editor and Visual Studio.
2. Backup your current project folder.
3. Extract `BMXFreestyleReborn_UE5_8_MOVEMENT_PATCH_v0_1_4.zip`.
4. Copy the `Source` folder into the **root of the existing BMXFreestyleReborn project** (the folder that contains `BMXFreestyleReborn.uproject`). Confirm **Replace** for the three matching source files.
5. Double-click your existing `.uproject` file and allow Unreal to compile.

## Playtest checklist
1. Click once inside the Unreal Play-in-Editor viewport to capture keyboard input. Hold W. The HUD should show `INPUT W/S +1.0`. If it stays at +0.0, the viewport did not receive W and this is an input focus/mapping problem, not ground collision.
2. On flat ground, W should move the bike **without needing to press Space** first.
3. Turn with A/D, brake/reverse with S. Observe `GROUND` and speed updating.
4. Approach a quarter-pipe gradually and pedal onto its surface. If the collision blocks forward movement, HUD should show `BLOCKED` and speed should slow, but turning/reversing should still respond.
5. Hop with Space and move into a ramp side/wall. The bike should descend instead of remaining indefinitely in midair.
6. Jump then perform B (barspin); land and verify +150 points are banked. Press R to return to the park entrance.
7. If a regression occurs, send a short video showing the new `INPUT` / `GROUND-AIR-BLOCKED` readout and the latest `%LOCALAPPDATA%\\UnrealBuildTool\\Log.txt` if compilation fails.

## Verification and limitations
- This is a **source patch**, not a precompiled executable.
- Source structure, changed-file inclusion, project configuration and archive integrity were checked; Unreal Engine 5.8 compilation and in-game physics testing are **not available in the build environment**. Further physics tuning may be necessary, especially for steep quarter-pipes and grinds.
- This is a focused stability patch. The existing placeholder rider and visual geometry are unchanged.
