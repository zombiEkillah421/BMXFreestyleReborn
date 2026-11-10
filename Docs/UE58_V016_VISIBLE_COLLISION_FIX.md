# BMX Freestyle Reborn v0.1.6 — Visible Ramps & Matching Collision

## What this update addresses

Your v0.1.5 playtest reported hitting objects that you cannot see, while visible ramps can be passed through. The v0.1.5 ramp generator used only a single, open strip of triangles without side walls or an explicitly assigned material. Its triangle winding was opposite the upward-facing normals, a likely reason for missing front faces and unreliable ramp collision.

This update rebuilds each ramp as **one closed procedural mesh**. The top riding surface, visible sides, back end and underside are all triangles from the same component. The component's complex-as-simple collision is cooked from these exact visible triangles. There are no added invisible ramp collision boxes.

## Changes

- Corrected top-face triangle winding and generated outward-facing side/end/bottom faces.
- Added 5 distinct runtime-tinted stock Unreal materials: dark floor, bright amber ramps, cyan ledges, silver rails, and visible dark perimeter walls.
- Kept the same v0.1.5 ramp positions, dimensions and intended ~60-degree quarter-pipe finish.
- Added component tags to the collision geometry (RAMP, FLOOR, LEDGE, RAIL, WALL).
- Added HUD blocker identification: when the bike says BLOCKED it also shows the component it collided with (for example `HIT: RAMP` or `HIT: WALL`).
- Added output-log lines prefixed `[BMX PARK v0.1.6]` for ramp collision audit and startup confirmation.
- Left the v0.1.5 movement, camera, input bindings, trick animations and scoring behavior intact except for the diagnostic readout.

## Install — Source patch

1. CLOSE Unreal Editor and Visual Studio.
2. Back up your current v0.1.5 project folder.
3. Extract the patch ZIP and copy the `Source` folder onto the `Source` folder in your `BMXFreestyleReborn` project. Choose Replace when asked.
4. Re-open `BMXFreestyleReborn.uproject` in Unreal Engine 5.8 and allow compilation.

**Do not put the patch folder inside the project as a second nested `BMXFreestyleReborn` folder.** The patch's top-level `Source` folder belongs next to the existing project's `Config`, `Docs` and `.uproject` file.

## First playtest

1. Confirm the HUD reads **`PARK 01 - v0.1.6`**. If it still says another version, the replacement did not take effect.
2. From spawn, approach the **amber curved quarter-pipe** while holding W. The colored surface should remain visible from above and the bike should not pass through it.
3. Move around the tabletop and ledge. You should encounter only obstacles visibly present in front of the bicycle.
4. If `BLOCKED` appears somewhere unexpected, note the HUD's `HIT:` value and capture a short recording. It identifies whether the bike touched a ramp, wall, ledge, rail, floor, or an untagged engine/world component.
5. Ride up a bank ramp, jump, and land a barspin. Tricking and scoring should work as before.

## Test status

Source/archive checks and independent triangle-winding/geometry validation can be performed here. Unreal Engine 5.8 compilation and actual in-game collision testing **cannot** be performed in this environment. The next in-engine playtest is required to confirm the fix.
