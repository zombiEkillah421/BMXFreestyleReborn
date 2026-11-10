# Changelog

## 0.3.2 — dirt line fixes

From the v0.3.1 playtest video (vert airs and manual combos working):
- The dirt spawn was about 1 m from the wall, so the camera was squashed into the rider. It now starts 5 m out with a **rolling start** at about 41 km/h, including after bails.
- The run-in from a standstill was too short, so you landed short in the gap. The rolling start fixes that.
- The landing backside was 45°, too steep to pedal up, so landing short left you stuck at its base. It's now 30° and rideable. 40–45 km/h lands the downslope, 38 km/h lands the deck, slower rides up and over, and faster lands on the flat run-out.
- The three sets were moved to fit the new start.


## 0.3.1 — playtest fixes (dirt jumps, manuals, flips, pumping)

From the v0.3.0 playtest video:
- **Dirt jumps / steep landings:** falling onto a steep face (dirt landing backside, ramp face) left the bike hanging in the air until "PHYSICS RECOVERY", because the body touched the slope before the wheels reached landing height. Touching any rideable slope with wheel support now counts as landing. Brushing a rideable slope in the air no longer scrubs speed.
- **Manual:** W held from pedaling dropped the nose instantly. W/S now only balance once you've released W after starting the manual, and the balance is calmer (instability 1.4 → 0.9, gentler wobble, correction 2.2 → 1.6). Tuning is under BMX|Tricks.
- **Accidental flips:** keyboard flips/spins are now only F/G and Q/E. W/S and A/D no longer rotate you in the air (the gamepad stick still does).
- **Pumping:** holding W while dropping down a transition adds speed (PumpAcceleration). Above pedal speed, drag is now light (HighSpeedDrag 60). Coasting drag is 240 → 140.
- **Vert energy:** the vert launch used sin(chord pitch), which threw away about 35% of the speed every air, so airs shrank. Now 70% of the speed shows as height (a full-speed first air is about 4 m), and dropping back in returns the launch speed (95%). Back-and-forth airs keep their height and grow when you pump. The speed cap is raised to 1.8x pedal speed.


## 0.3.0 — freestyle core

**Tricks you can combine**
- Free air rotation replaces the fixed flip/spin buttons: W/S (or F/G) flip, A/D (or Q/E) spin. Any amount, so 180s, 360s, 540s, backflips, frontflips and doubles all happen naturally. When you let go near a full turn the rotation settles onto it.
- Flips and spins layer with every bike/body trick, so a backflip tailwhip or a 360 superman works.
- 10 style tricks: barspin, tailwhip (now swings round the head tube), no-hander, superman, can-can, X-up, turndown, tabletop, nothing, tuck no-hander. Hold tricks keep scoring while held. Landing while still holding one is a "tied up" bail.
- Landings are judged on rotation: within 50° of a whole flip and a half spin. Landing a 180 rides away fakie.
- Fixed: the old "backflip" actually rotated forward.

**Manuals and combos**
- Hold Shift on the ground to manual, keeping the balance meter centred with W/S. Too far back loops out, too far forward drops the nose.
- Land holding Shift (or press it within 0.3 s) to keep the combo going. Hop out of a manual and the combo carries into the next air.

**Riding**
- Charged bunny hop: tap for 0.9x, hold Space up to 0.35 s for 1.3x height (about 1.6 s of air on flat).
- In the air A/D now spin instead of steering.
- Only quarter-pipes give vert airs. Ramps are tagged, so steep dirt lips launch you forward.

**Park**
- Three-set dirt jump line along the south edge (press 2 to start there, 1 for the park). Each set has a 1.3 m lip, a 2.8 m gap and a long landing. About 36–45 km/h lands on the downslope, checked with a 2D trajectory calculation.

**Rider model**
- New hip and shoulder joints so poses move the torso and arms.

Not yet compiled in UE 5.8: run `BUILD_EDITOR.bat`, see `Docs/V030_PLAYTEST.md`.


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
