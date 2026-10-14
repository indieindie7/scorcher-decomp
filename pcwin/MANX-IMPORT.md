# Manx TT handling in SCORWIN: what we built and what we learned (2026-10-13/14)

Status: **paused** (user, 10-14: "imma stop the racing game making for now, maybe in the future i will come back to this").
This project is research and modding only, and it is non-commercial. This repo holds our own code and notes. It holds no
game files, no disc images and no extracted data: every Manx TT asset is converted locally from the user's own disc.

## Goal
Bring Manx TT Super Bike's (Saturn) bike, rider and handling into Scorcher's Win32 build (SCORWIN.EXE). We hook it
through our own `ddraw.dll` proxy (`pcwin/ddraw/ddraw.c`). This was a step toward a cyberpunk bike/car racer, a
spiritual successor to Scorcher. The Manx TT reference work (decompile and study notes) lives in the separate
manxtt-decomp project.

## How it is wired (pcwin/ddraw/ddraw.c)
- **Bike select screen** before the race, Daytona style (`draw_bikesel` / `bikesel_run`). The choice is saved in the ini as `[game] manx`.
- **Bike model:** the Manx bike is converted to Scorcher's world format ("ting" .wl3 + textures) by `tools/manx2ting.py --pc`.
  - Colour 0 is transparent on the Saturn's VDP1 but drew magenta in SCORWIN. Fix: A = the Manx texture, B = an all-white layer.
- **Physics** (`manx_post`, hooked after the player tick, `bilstyring_RateLoop` 0x41b05c):
  - speed, gears, lean and the Manx steer-through-lean;
  - landing squat (`manx_squat`);
  - catch-up (`manx_catchup`);
  - reverse (`manx_reverse`).
- Scale: `manx_scale` 4400 world units per metre.
- **Full Manx player** (`manx_full`, default 1). Hooks on the player tick:
  - `car_DampVelocity` (call at 0x41b0f5) is skipped;
  - `car_CollideStep` (0x41b131 -> 0x417d5d) is wrapped. Instead of Scorcher's bounce, the Manx wall rule applies: speed
    keeps only the share of the move that got through, and a hard hit starts the crash state;
  - `car_GroundResponse` (0x41b15a -> 0x416be8) still counts ground ticks, but its velocity change is undone.
- **Visuals:**
  - the bike leans: a hook on `car_BuildWorldMatrix` at 0x41b175;
  - the rider plays a pose per lean, plus idle poses (`manx\rider.bin`, format MXR2, built locally);
  - the wheels spin;
  - Manx chase camera;
  - Manx engine sound mix.
- **Single-track model** (`manx_model 1`, the "successor" experiment): lean picks the bar angle through a rider law, and
  2-DOF yaw/side dynamics use magic-formula tyres. Tuning keys: `manx_yawi`, `manx_us`. Based on the Meccanica 2022 paper
  in `bike_physics_papers.md`.
- **F2 collider view** (F1 is the options menu): draws the player's collision volume in game.
- **Wheel colliders** (`manx_wheelcol 1`, `manx_wheelr` 1500): see below.

## Discoveries
- **Scorcher's vehicle is ONE swept sphere.**
  - `car_CollideStep` -> `col_Trace` sweeps it from the old position (wram+0..8) to the new one (wh+0x20).
  - Its radius is in the shared record, `*(*(wh+0x98)+0x10)` = **64000** for every car (player, enemies, barrels).
  - At our bike scale that is about 14.5 m: roughly the width of the road. The F2 view showed this.
  - The road surface is not hit by the sphere (the sphere cuts through it); the ground is handled separately (`car_GroundResponse`).
- **Manx TT bikes are not even a ball** (from the manxtt-decomp handling study):
  - **Against the track**, the bike is a single point, moved in 10 substeps (`bike_SubstepMoveCollide` 0x06037238).
    Each substep finds the course polygon under the point. If that polygon's surface flags mark it as blocked, the move
    is undone and speed goes to 0. There is no bounce.
  - **Between bikes**, the test is a flat octagon on X/Z only (`car_FindNearbyCars` 0x0602cb68):
    3·max(|dx|,|dz|) + min(|dx|,|dz|) < 43 units. It marks the neighbour and drives crash state 3.
- **Two-wheel colliders:** run `car_CollideStep` twice with both points moved ±0.7 m along the heading and a small radius,
  then merge the two push-outs per axis.
  - Gotcha: save and restore only wram +0..+0x14 (old position and velocity) between the two calls. The rest of the record
    holds list links. Restoring a 0x100-byte snapshot crashed `list_Begin` (0x4022f4).
- **Single-track model bug (open):** in game, yaw rate ran away (r reached -1.64 rad/s at 25 m/s, lean pinned at 60°).
  Suspect: the side-speed readback uses the matrix base at +0x40, while the heading axis is base 1 (+0x4c).
  The offline sim (0.8 to 0.94 g of grip, turns settling in 0.2 to 0.36 s) was fine.
- **Gear stuck in 1st (open):** rpm tops out at about 8400 and the gearbox never shifts up.
- Why few sim bike games exist: tyres, lean and collision all interact. The Manx-style shortcut (a point, polygon flags,
  and stop dead on walls) is a big part of why arcade games feel "fair".

## Not done when paused
1. Test the wheel-collider build (it crashed once; fixed but not yet run).
2. Fix the single-track yaw runaway.
3. Fix the gear stuck in 1st.
4. Check the colours of texture fix 2.

## ini keys ([game] in scorcher.ini)
| Key | What it does |
|---|---|
| `manx` | Use the Manx bike |
| `manx_full` | Full Manx player |
| `manx_model` | 1 = single-track model, 0 = Manx steering |
| `manx_yawi`, `manx_us` | Single-track tuning |
| `manx_tyre` | Tyre model |
| `manx_scale` | World units per metre |
| `manx_reverse` | Reverse |
| `manx_squat` | Landing squat |
| `manx_catchup` | Catch-up |
| `manx_colr` | Single-sphere radius, 0 = stock 64000 |
| `manx_wheelcol`, `manx_wheelr` | Wheel colliders and their radius |
| `manx_colflip` | Mirrors the F2 view if it draws on the wrong side |

The full dated log, with addresses, backups and the user's requests, is in NOTES.md (the "Manx" bullets at the end).
