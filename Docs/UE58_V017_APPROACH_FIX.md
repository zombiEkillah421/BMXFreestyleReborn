# BMX Freestyle Reborn — UE5.8 v0.1.7 ramp approach patch

## What the recording revealed
The v0.1.6 HUD is present. At about 12 and 21 seconds of the November 8 recording, the rider contacts a broad orange wall or HUD `HIT: RAMP`. The quarter-pipe geometry had the low (ride-on) end near the perimeter and the tall rear face toward the middle of the training park. Those are solid rear walls; the bike could not ride over them from the center.

## Targeted changes
- Turn four quarter-pipes to face the central riding lanes; keep the same closed visible/collision meshes.
- Shift their origin points so their low lips are at x=-690 and x=+690, without disturbing the tabletop or rails.
- Move both spawn and reset to x=-150, y=-1150, facing west: a clear approach toward the left quarter-pipe.
- Narrow the pawn collision radius from 29 to 24 cm to reduce minor edge catches without removing solid obstacles.
- Version and lane hints on the HUD.

## Installation
Back up your existing project, close Unreal Editor, and copy the patch Source folder over the Source folder of the project you **actually launch**. An earlier log located that project under `C:\Users\richa\Documents\BMXFreestyleReborn 5.8`, not the former Desktop path. If that path has since changed, use the folder containing your currently opened `.uproject`.

Rebuild by reopening the `.uproject`. The HUD must read `PARK 01 - v0.1.7`.

## Test
1. Hold W from spawn: this now faces the low, curved entry of the left-hand ramp.
2. Climb the orange surface, turn, and ride back down.
3. Try both quarter-pipes from the center; watch the `HIT:` readout.
4. Tall back walls, ledges and rails remain solid obstacles. They must be approached from rideable sides or hopped over. No trick/combo code was changed.

## Verification limits
ZIP validity and source-oriented geometry assertions were tested locally. No Unreal Engine 5.8 compiler or real-time physics simulation is available in this sandbox; the gameplay outcome requires a new playtest.
