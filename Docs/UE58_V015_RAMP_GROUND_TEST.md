# BMX Freestyle Reborn v0.1.5 — UE 5.8 ramp / grounded-state update

## Based on your 2026-11-08 gameplay recording

The v0.1.4 HUD shows input arriving (`INPUT W/S +1.0`), a `BLOCKED` state with 0 km/h near an obstacle, and an unnecessary `PHYSICS RECOVERY - RESPAWNING` message during ordinary riding. A successful barspin eventually banks 150 points, so trick scoring still works.

## Source changes

- Quarter-pipe geometry now transitions smoothly to a ~60-degree curve parameter, rather than a vertical end. Corrected surface normals for the revised geometry.
- Anticipatory slope probes allow larger steps only on genuine ramps, not flat raised ledges; front-edge clearance reduces capsule snagging on the ramp.
- Increased slope down-step tolerance and aligned walkable landing threshold with ground-trace threshold.
- When settling against an obstacle, try nearby supported ground before entering freefall. Before any automatic physics reset, attempt supported-ground recovery.
- HUD speed and generated wheel rotation now use actual horizontal travel, so a wall cannot display fake high speed.
- Retained the existing input bindings, bike appearance, follow camera, tricks, scoring and park layout.

## Install small patch

1. CLOSE Unreal Editor and Visual Studio.
2. Back up your working `BMXFreestyleReborn` project folder.
3. Extract the patch's `Source` folder into your existing project folder, overwriting only matching files.
4. Open `BMXFreestyleReborn.uproject` and allow Unreal Engine 5.8 to rebuild.

## Test in this order

1. Without pressing Space: hold W and check the rider moves, and the HUD speed rises.
2. Ride into open floor, brake, reverse, and steer. `BLOCKED` should only be momentary when contacting a solid obstacle.
3. Ride directly up one of the *curved* quarter-pipes. Watch for ramp snagging, false `AIR` or automatic `PHYSICS RECOVERY` messages.
4. Hop off a ramp and land. `GROUND` should return after landing.
5. While airborne press B for barspin. Landing should bank 150 points if the trick completes.
6. If still stuck, press R, record the last few seconds including `INPUT W/S` and `GROUND/AIR/BLOCKED`, and send the new log if compilation failed.

**Important:** This is a source-level correction, not a packaged Windows game binary. Neither a UE 5.8 C++ compile nor runtime physics test is possible in the current environment; do not treat these fixes as validated in-engine until the next user playtest.
