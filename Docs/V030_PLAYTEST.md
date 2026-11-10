# v0.3.0 playtest — freestyle core

1. Close Unreal. Run `BUILD_EDITOR.bat` and wait for BUILD SUCCEEDED. Then run `OPEN_PROJECT.bat`.
2. The HUD should read **PARK 01 - v0.3.0**.

If the build fails, send `BuildLog.txt`.

## Try these

1. **Big hop:** on the flat, hold Space for half a second and let go. It should be noticeably higher than a tap.
2. **Flip off a hop:** big hop, let go of W, then hold S. The rider should backflip. Let go near upright and the flip settles onto the landing.
3. **Combine:** big hop, hold S (backflip) and tap T (tailwhip) at the same time.
4. **Spin:** big hop, hold D. Let go around half way to land a 180 and ride away fakie (the HUD says FAKIE).
5. **Hold tricks:** in the air hold K (superman). Let go before you land, or it's a "tied up" bail.
6. **Manual:** ride at speed and hold Left Shift. Keep the white marker in the green with W/S.
7. **Combo link:** hop, do a trick, land **holding Shift** into a manual, hop again, do another trick, land. One combo with a growing multiplier.
8. **Vert:** quarter-pipe at speed. During the vert air press K or hold S for a flip.
9. **Dirt line:** press 2. Pedal to about 40 km/h and hit the first lip. Try a no-hander (J) over the gap.

## Please report

- Anything that bails when it shouldn't, and the message it shows.
- Whether flips and spins rotate too fast or too slow (`FlipRateDeg`, `SpinRateDeg` under BMX|Tricks).
- Whether manuals are too easy or too hard.
- Whether the dirt jumps feel right at the speed you naturally hit them.
