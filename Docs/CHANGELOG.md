# Changelog

## 0.2.0 — slope physics and vert airs

**Riding**
- Speed now means speed *along* the riding surface. Only its horizontal share is applied on slopes, so climbing a ramp no longer makes the bike faster.
- Slope gravity: going uphill slows you, downhill speeds you up, and stalling on a quarter-pipe rolls you back down fakie.
- Pedaling is weaker on steep faces, so quarters have to be hit with speed carried from the flat.
- Holding W while going faster than pedal speed (e.g. after dropping back in) no longer brakes the bike.
- Pedaling in the air no longer changes speed.
- Launches split surface speed into horizontal and vertical parts. Before, the full horizontal speed was kept *and* lift was added.
- Landings keep momentum by projecting the air velocity onto the landing slope.
- The bike levels out gradually during ordinary jumps.

**Vert airs**
- Quarter-pipes are steeper: the arc parameter went from 45° to 60°, so the tops finish at about 58–62°.
- Leaving a quarter-pipe steeper than 30° (or hopping near its top) is now a vert air. The rider goes straight up, turns 180° in the air, drifts back over the ramp and lands facing down the transition.
- A quarter-pipe lip now launches at any rolling speed above 150 cm/s. The tabletop deck still needs 525+.
- Tuning is exposed under BMX|Vert: launch pitch, launch scale, min/max lift, drift distance.

**Scoring / HUD**
- Repeating a trick in the same combo scores 100% → 50% → 25%…
- The debug readout toggles with Tab or the gamepad View button. A "VERT AIR" state was added.

Verification: the core ramp math was checked in a 2D simulation of both quarter sizes (full-speed vert launch and landing, coasting stall and fakie). It hasn't been compiled or playtested in Unreal yet. See `Docs/V020_PLAYTEST.md`.

## 0.1.4 – 0.1.9 — ramp and collision fixes (UE 5.8)

- 0.1.4: movement stability; separate horizontal/vertical sweeps; INPUT / GROUND / AIR / BLOCKED HUD readout.
- 0.1.5: smoother quarter-pipe geometry; slope-aware probes; supported-ground recovery.
- 0.1.6: ramps rebuilt as closed meshes with matching visible collision; tinted materials; `HIT:` blocker readout.
- 0.1.7: quarter-pipes turned to face the riding lanes; new spawn point; smaller collider.
- 0.1.8: two wheel-contact probes replace the single center ray; compact collider.
- 0.1.9: open-lip quarter-pipe takeoff; pitch kept during rear-wheel-only contact; wheel contact shown on the HUD.

## 0.1.0 — first UE5 source prototype

- Implemented the training area generated at runtime: curved ramps, banks, tabletop, ledge, rails, perimeter and light.
- Implemented the arcade BMX pawn: riding, steering, jumps, landing tests, camera orbit, reset and bail recovery.
- Implemented 6 air tricks, a sequential combo queue and a score multiplier.
- Added keyboard/gamepad bindings and a HUD with no external UMG assets.
