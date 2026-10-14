# Racing/vehicle physics compared (from notes only)

Sources: Scorcher = `Scorcher/NOTES.md` (lines 356-530, 1052-1070). Manx TT = `ManxTT/ghidra/study/study_handling.md`, `study_ai_camera.md`, `study_chasecam.md`. Sonic R = `SonicR/study_handling.md`, `study_ai_camera.md`. Cyber Speedway = `CyberSpeedway/NOTES.md`. Cyber Sled = `CyberSled/NOTES.md`. Amok is a shooter, not a racer, so it was skipped. GUESS marks guesses made in the notes themselves.

## 1. Table

| Game | Tick / fixed point | Speed | Steering | Grip / skid | Ground / air | Collision | AI / rubber band | Camera | Key functions |
|---|---|---|---|---|---|---|---|---|---|
| Scorcher (PC) | 33 ms fixed (~30 Hz); 16.16 | engine value, accel curve, per-axis damping | keys, +4/tick ramp, clamp 0x200 | slide keys add side force; no skid model decoded | gravity +0x1200/tick; jump energy | substep sweep; car-car restitution 0.4; surface bounce 0.19 | AI shares player physics; rubber band not decoded | moincam, vel damping 3/4 | car_PlayerDriveStep 0x29d9f, car_ApplyControls 0x24cbc, car_CollideStep 0x26aef |
| Manx TT (Saturn) | 1 tick per frame (25 PAL / 30 NTSC), dt 0.04; 16.16, 12-bit angles | torque x gear x efficiency; Cd drag | cubic input, lean spring-damper | lateral vel damped; slip only for sound | 2 probes (rear, front 14.0 ahead); air if gap > 1.5696: height holds, no gravity / vertical velocity, snaps back to ground; pitch chases atan2(rear-front ground, 14.1) by 0.3/tick, +227 (~20 deg) nose-up kick off crests | 10 substeps, speed zeroed on hit | two-way rank multiplier (behind 1.10-1.23, ahead ~0.9); catch-up pull | (0,-4,-26) behind pivot, yaw lag 1/8 | bike_UpdateRacerPhysics, car_UpdateByState 0x0602d2c8 |
| Sonic R (Saturn) | tick unknown; 12-bit fixed, angle 0..0xFFF | per-character table, rubber scale | digital, heading += steer rate (25-45) | fwd/lat velocity, lat decays by friction | quad grid; g 3.0/frame; bounce | swept 16 substeps; wall reflect ~1/3 | CPU stat scale 0.5x-2x by gap | 896 behind, +0x70 up, lag 1/4 xz, 1/9 y | car_UpdatePhysics 0x0602d2a0, ai_UpdateRubberBand 0x06035974 |
| Cyber Speedway (Saturn) | step D=2 (~30 Hz, GUESS; D=3 in mode 0x8002); 8 fractional bits, 1024 units/rev | speed scalar 0..255, +1..3/update, brake -6..-12, no drag, vt=speed*0.95-1.05 | digital ramp +8..16/update to +-144, heading += steer*D/16, not speed scaled; lean = steer/4 | single velocity vector blended v=(t*(256-g)+v*g)>>8, g 64..241 by slide angle = drift | spring hover to tile height, air gravity 1/update to -16 | wall: head-on loses speed, damage, bounce impulse; car-car radius 40, force<=50 | waypoint AI, per-slot tables, rank-based catch-up (GUESS) | not decoded | car_UpdatePlayer 0x06020074, car_UpdatePhysics 0x0602f79c |
| Cyber Sled (arcade) | unknown / not decoded | unknown / not decoded | unknown / not decoded | unknown / not decoded | unknown / not decoded | unknown / not decoded | unknown / not decoded | unknown / not decoded | none named |

## 2. Per game

### Scorcher (PC build SCORDOS; addresses are SCORDOS)
- Tick: `ctx+0x138 += 0x21` (33 ms, fixed rate ~30 Hz); RateLoop for all worlds, then Frmupdate.
- Player and enemy cars share physics. `bilstyring_RateLoop 0x29d71` calls `car_AIDriveStep 0x2a370` or `car_PlayerDriveStep 0x29d9f`; `enecon01_RateLoop` calls `car_DriveStepAI 0x29eba`.
- Order per tick: `car_DampVelocity 0x24563` (car-space scale 0.945, 0.992, 0.996), `car_ReadKeys 0x29c17`, `car_ApplyControls 0x24cbc`, gravity (vel y `wram+0x10` += 0x1200), `car_UpdateSpeed 0x24699`, `car_Integrate 0x246c5`, `car_CollideStep 0x26aef` (n = 6), `car_GroundResponse 0x2597a`, `car_UpdateTilt 0x27064`.
- Speed: engine `+0x130`. `car_AccelCurve 0x24af1`: ratio < 0 gives 1500; boost gives min(timer,200)*20; ratio < 61 gives (60-r)*83+1000; < 126 gives (125-r)*6+250; < 201 gives 100; else 0. `car_BrakeCurve 0x24b73`: 1500 forward; reverse (40-r)*100+400, then 200, then 0. Ratio = fixmuldiv(speed, power `+0x13c`, 0x1ce80). Nitro (Alt): engine += 3000 up to 0x1e848, energy `+0xdc` -= 0x280. Race start: boost timer 10, strength 200.
- Steering: `+0x168` += 4 per tick held, steer = +-(`+0x168` + 0x90), clamp 0x200; smoothed `+0x16c` = (old*7 + new*3)/10; `car_ApplySteerYaw 0x247ba`. No return-to-centre spring. Slide keys add -+0x2800 along the side vector.
- Jump: `car_Jump 0x25339`, 10 ticks (~0.33 s), impulse table `DAT_000a7584` (0.4, 0.2, 0.1 ... 0.0008), energy -= 0x2000 per tick; `car_Respawn` refills `+0xd8` >= 0xa000 and `+0xdc` >= 0x2aaa.
- Collision: radius 64000, mass 500 (`shared +0x10/+0x14`). `phys_CollideSpheres 0x25bcf`: centre-of-mass frame, restitution 0x6666 = 0.4, minimum push 100. Surfaces via `vec_ReflectDamped 0x23b1c`: 0x3000 = 0.19 bounce, 0xfe00 = 0.992 and 0xf400 = 0.953 tangential. Landing from the air adds fixmul(0x13cc, 1-t).
- AI: `car_AIComputeSteer 0x289b9`; avoidance via `car_ForEachNearbyCar 0x28366`, radii 0x1f400 / 0x5dc00. Rubber band: unknown / not decoded.
- Camera: moincam chase; `moincam_FollowTick 0x389d6`; in-car offset (0,-0xe100,-0x7d00).

### Manx TT Super Bike (Saturn; base 0x06010000)
- dt 0x0A3D = 0.04 s: one physics tick per rendered frame (frame divider 2 = 30 fps NTSC / 25 fps PAL), no dt compensation, so 0.04 is exact only on PAL (drill pass 10-13). g = 0x9CF5C = 9.81. 10 collision substeps.
- Accel = F*9.81/(mass+12); mass 237.0 + 60.0 rider. Gears 2.4, 1.94, 1.63, 1.43, 1.29, 1.19. rpm clamp 1300-15000, upshift 13000, downshift 10000, 26-tick shift delay. Brake 500/618/481/437/600/562 kgf. Cd 0.502 or 0.3765. Top speed emergent (not a stored constant).
- Steering: `bike_UpdateSteeringTorque 0x060384CC` (cubic, clamp +-0x1A, x56.4 x0.04); `car_UpdateLeanBalance 0x06031F14` spring 300.0, damping 0.51, lean = -atan2(`rec+0x124`, 1.41); heading clamp +-0x33C.
- Off-road: surface type 1 x0.9, type 3 x0.6. Crash states 3/4 damp 0.98 per tick, otherwise 0.91.
- Rubber band (corrected, drill pass 10-13): works both ways. Reference = the bike one rank ahead (the leader uses the player). Behind: speed multiplier 1.10-1.23 (set 0; up to 1.98 in set 2); ahead: 0.925-1.1 (default 0.9); table 0x0606EAAC, 3 sets x 29. `car_ComputeCatchupBonus 0x060330e0` pulls every non-leader bike (player too) toward the bike ahead within 600 units, cap 80.0, speed gate 5.0 (not 80). AI: `ai_SelectSpeedTarget 0x06034e80` (table {base, slope}, 29 entries); `ai_UpdateRacerRubberBand 0x06035104`; `ai_UpdatePathFollow 0x060346b4`; cone test +-97.
- Slope (10-13, HANDOFF-scorcher.md "Slope pitch and gravity"): 9.81 is only a unit factor, there is no gravity in the air. F_slope = m*nz' - 0.012*m*ny' - 0.0334*Cd*v^2 with the rear normal in bike space (ny' ~ -1, so the 0.012 term is a small constant forward push, not rolling resistance; drill pass 10-13); after 5+ air ticks only drag acts. bike_UpdateRacerPhysics 0x0602f7d0, car_UpdateThrottleSpeed 0x06032918. Crash = touching another bike (3*max(|dx|,|dz|)+min < 43).
- Final pass (10-13): crash recovery 3 ticks (rec+0x154); AI shifts 14000/7000 vs player 13000/10000; collision shoves decay 1%/frame; AI speed table at 0x0606EAAC; lean atan2 constant 1.41 = 0x168F5; look-ahead dead in the shipped game.
- Landing (drill pass 10-13): rec+0xd8 = vertical delta + 0.3924 gravity step per tick; first grounded tick rec+0xf0 = d8 x mass; car_UpdateLeanBalance decays it by mass x 0.0392/tick into a squat spring rec+0x128, used only as a draw offset (bike/rider, first-person camera Y). Visual suspension only: no speed loss, no sound; position Y still not integrated with gravity.
- Camera states: 2 first person (default), 3 near chase (also the far result cam), 4 a 7-frame roll/blend back to 2, 5 intro flyover, 6/0xe finish look-at; view button 2->3->4->2. Ghost: one 0x18-byte sample per frame, max 3000.
- Camera: offset (0,-4.0,-26.0) from pivot (0,-12.0,+7.0); yaw lag 1/8; pitch lag 1/18.048; pull-back speed>>3; focal 350 or 232 (1P), 180 (2P).

### Sonic R (Saturn; base 0x06004000)
- 12-bit fixed, angle 0..0xFFF. Tick rate: unknown / not decoded.
- `car_UpdatePhysics 0x0602d2a0`: steering adds steer rate (25 to 45) to heading; forward and lateral velocity kept separately; drift flag `+0x6e` = -1/0/+1.
- Character table at 0x0603aee8, 10 rows x 10 ints; row 9 highest top speed (0x3c000), row 3 lowest (0x24000); column meanings partly GUESS.
- Gravity 3.0/frame (0x3000), terminal fall 0x25800 = 37.5. Bounce `car_UpdateVerticalBounce 0x0602fa1c`.
- Walls: `car_ResolveWallCollision 0x060308e0`, `car_ReflectOffWall 0x06030480` (normal ~1/3); soft walls x3/4. Car-car: `car_CollideCarsSwept 0x06034b40` (16 substeps, radius 0x1b6 = 438), `car_BounceCarsApart 0x060349a4` (14/16 damping, clamp 7..0x14), `car_CollideCars 0x0602e32c` (push apart under 0x60).
- Rubber band: `ai_UpdateRubberBand 0x06035974`, T per course 0x4e/0x66/0xc8/0xc8/0x31; scales B/2, 7/8 B, B, 3/2 B, 5/4 B; cap 0x20..0xc8; `car_ApplyRubberBandStats 0x06035840`.
- Camera `cam_UpdateFollowSmooth 0x06019038`: 896 behind (1200 for slot 4), +0x70 up; lag x/z 1/4, y 1/9; no look-ahead.

### Cyber Speedway
Full detail: `CyberSpeedway/NOTES.md` "Movement physics (10-13)". MAP1_1 loads at 0x06012000; human car record in car_UpdatePlayer (0x06020074), AI/other slots in car_UpdatePhysics (0x0602f79c, SoA, 13 slots).
- Units: 1024 angle units/rev, Q15 trig, positions 8 fractional bits (x axis inverted). Step multiplier D=2 (3 in mode 0x8002); ~30 Hz is GUESS.
- Speed is a 0..255 scalar: accel +3/+2/+1 per class (x Dacc), brake -12/-9/-6 (x D/2), coast -1.5*D, no drag. Velocity magnitude = speed*(95/100/105)*D/512 per class, thrust scaled x0.75.
- Steering: digital ramp +8/+12/+16 per update to +-144; heading += steer*D/16 (max 18 units); not scaled by speed. Visual lean = steer/4/D.
- Grip/drift: one velocity vector, v = (t*(256-g) + v*g)>>8 with g from a table by slide angle (class0 110..225, class1/2 64..241). Low slide gives g 64 (loose), bigger slide gives more grip, so the car self-recovers. Lateral velocity is implicit, no separate lateral state.
- Side-dash buttons add a decaying sidestep impulse (>>2 per update) costing energy.
- Hover: spring vz += (ground-z - vz)*k/64 (k 16..20), vz damped 1/8, air gravity 1/update to terminal -16, airborne if dz-vz < -20.
- Walls: heading-vs-wall factor, speed loss 128-factor, damage = loss*speedmag/2048, bounce impulse 0x40..0x80. Car-car (AI slots): radius^2 < 1600, |dz|<10, force min(dist+10, 50).
- AI: waypoint follower with per-slot tables and a rank comparison (catch-up, GUESS). Unknown: tick, mode 0x8002, boost, analog steer, car_CollideCars.

### Cyber Sled
No physics, speed, steering or collision decoded; the notes cover ROM layout, function discovery and naming only.

### Amok
Not a vehicle game (1996 shooter); not read for physics.

## 3. What Scorcher could borrow
- Manx steer-to-lean spring-damper (300.0 / 0.51) to give Scorcher's +4/tick steer ramp a self-centering return, which it lacks.
- Manx cubic steering shaping and a speed-scaled steer clamp on top of the 0x200 limit.
- Sonic R forward/lateral velocity split with a grip factor, so the slide keys become a real drift with per-car grip.
- Sonic R per-car stat table (~10 numbers) feeding Scorcher's power, mass and radius.
- Rubber band: Sonic R's single 0.5x-2x scale on AI stats by progress gap (floor and cap), or Manx's two-way rank multiplier (behind 1.10-1.23, ahead ~0.9) plus a catch-up pull (gate 5.0). Scorcher has none decoded.
- Sonic R wall response (slide glancing, reflect ~1/3 head-on) versus Scorcher's 0.19 bounce / 0.992 tangential; swept 16 substeps for car-car.
- Manx two-probe pitch plus a 1.5-unit airborne flag for crest attitude.
- Manx surface multipliers (x0.9, x0.6) for off-road, and speed pull-back plus lagged yaw (1/8) on the camera.
