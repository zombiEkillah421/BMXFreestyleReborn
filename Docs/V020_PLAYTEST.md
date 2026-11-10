# v0.2.0 playtest — slope physics and vert airs

Pull the update, reopen the `.uproject` and let it compile. The HUD should read **PARK 01 - v0.2.0**.

If compiling fails, copy the first error line from the Output Log or Visual Studio's Error List and send it.

## Test in this order

1. **Flat riding.** Hold W from spawn. You should reach full speed (about 49 km/h) on the flat like before.
2. **Coasting up a quarter.** Get up to speed, let go of W and roll at the amber quarter-pipe. The bike should slow down as it climbs, stop, and roll back down backwards. The HUD should show a positive SLOPE value.
3. **Vert air.** Hold W and ride straight up a big quarter at full speed. Expect `VERT AIR!`: the rider rises about 2.5 m, turns around in the air and lands on the ramp facing down, then rides away with speed.
4. **Vert + tricks.** During a vert air, press B or G. A backflip (0.83 s) fits inside a full-speed vert.
5. **Angled approach.** Hit a quarter at about 45° to its face. Note whether it vert-airs or flies off the back.
6. **Tabletop.** Jump the tabletop at full speed. It should still be a normal forward jump, not a vert.
7. **Hop on a slope.** Hop near the top of a quarter (vert) and on the flat (normal hop).
8. **Repeat penalty.** In one air, do B then B. The second barspin should add 75 points instead of 150.
9. **Tab** hides and shows the debug lines.

## Please report

- Did the vert land cleanly, or did it say `BLOCKED`, `PHYSICS RECOVERY` or `INCOMPLETE TRICK`? Include the `HIT:` value if shown.
- Is the vert too high, too low, or about right? (`MaxVertUpSpeed` / `VertLaunchScale` change it.)
- Is going up ramps now too slow or too fast? (`SlopeGravityScale` changes it.)
- A short recording is ideal.

## Quick tuning without code

Select the rider during Play (or eject with F8) and edit the **BMX|Tuning** and **BMX|Vert** values in the Details panel. These changes reset when you stop Play. Tell me the numbers you like and I'll make them the defaults.
