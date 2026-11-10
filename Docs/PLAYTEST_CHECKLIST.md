# Real Unreal Editor playtest — v0.1.0

**Status: not yet executed in Unreal.** Mark items pass/fail only after running in the Editor and watching behavior.

## Startup
- [ ] UE5 opens the .uproject and completes native code compilation.
- [ ] Opening Entry (or an empty map) and clicking Play displays a visible park and bike.
- [ ] The pawn is player-controlled and the on-screen HUD appears.
- [ ] Mouse and controller can be captured by the Play viewport.

## Core movement
- [ ] W accelerates, S brakes/reverses, A/D steer independently of camera.
- [ ] Letting go of W produces predictable rolling drag.
- [ ] Riding at full speed into ramps does not clip, flip, or freeze unexpectedly.
- [ ] Curved quarter-pipe collision matches the visible surface.
- [ ] R resets bike location and pitch, regardless of current state.

## Jump/landing
- [ ] Space jump from flat and from the start of a ramp produces visible lift.
- [ ] Jump cooldown prevents instant false landing.
- [ ] Landing detection registers only while moving down toward ground.
- [ ] Leaving the edge of an elevated platform results in a fall, not a teleport.

## Tricks and scoring
- [ ] B spins handlebars; T spins the frame; Q/E spin rider; F/G flip rider.
- [ ] Completed trick banks only on a safe landing.
- [ ] Queued combos display correct multiplier, and increment total only once.
- [ ] Landing before a trick finishes loses unbanked points.
- [ ] R resets an unfinished combo without removing banked score.

## Camera and presentation
- [ ] Mouse moves orbit camera without steering the bike.
- [ ] C recenters behind direction of travel.
- [ ] Camera stays usable on ramps and near walls.
- [ ] HUD text is legible at 1920x1080 and 2560x1440.
- [ ] Xbox controller inputs agree with documented mapping.

## Next engineering iteration
- [ ] Fix any Unreal compile errors with exact Output Log traces.
- [ ] Tune ramp shape, rider capsule and ground-probe behavior from in-game video.
- [ ] Replace primitive meshes with original BMX/rider assets, skin rigs and animation.
- [ ] Add manual/rail grind system and richer score state machine.
- [ ] Save an empty startup map and package one Windows build.
