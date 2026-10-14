# Handoff: native build work from the main Scorcher chat -> Scorcher modding session (2026-10-11)

You own native\saturnkit, build\recomp and build\recomp-build from now on. This is everything the main chat tried, so you do not repeat it. Exact values below were taken from NOTES.md "Native build step 1-8" (mine) next to your "pass 1-5".

## Toolchain I used (parallel to your MSYS2 one)
- LLVM 23.1.3 by winget (C:\Program Files\LLVM\bin\clang++.exe), VS 18 cmake + ninja, SDL3 3.4.18 VC zip in native\deps\sdl3-dl\SDL3-3.4.18, Khronos GL\glcorearb.h + KHR\khrplatform.h in native\deps\khronos (host.cpp needs them; -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS for std::max vs the Windows macro).
- native\build.sh does the whole cmake/ninja build and copies SDL3.dll; native\run.sh = fullscreen run; native\run.bat = the same for a double-click (no Git Bash). Either toolchain works; do not mix the two build dirs.

## Emit
cd native; set PYTHONPATH=C:/Users/john/Documents/Scorcher/native (the cwd drifts in long shells);
py -3.13 -m saturnkit.recomp --out build/recomp --overlay CDCON ZERO=build/extract/0@06010000 LORT=build/extract/LORT.BIN@06004000+0600B710 INTRO=build/extract/INTRO.BIN@00280000 CDCON=build/extract/CDCON.BIN@00200000
(plus your --landing LORT:0600D0C8). Seed +0600B710 only exists because the scheduler rte-s into it; with your sat_interrupt loop it is still needed (sh2_lookup must find it).

## What I found and fixed (both already in the shared clone)
1. CDCON must be an overlay (LORT calls 00200000 from pr 0600615A and expects a return).
2. vdp1_HwInit 0600A254 loops until (MODR & 0x178) == 0x10; the runtime never updated MODR. Patch in runtime/vdp1.cpp reg_write (comment "MODR mirrors the mode bits"): MODR = 0x1000 | TVMR low 4 | FBCR bits 1-4 at 4-7 | PTMR bit 1 at 8. Keep it when you rebuild.
3. The scheduler (same conclusion as your pass 3): 0600AABC VBlank-OUT handler -> callback *06016AB4 = 0600B848 -> rte into 0600B710 saving r15 at 06016AC0; state word 06016ABC (KLAR 4B4C4152 = idle, 1 = running, 0 = frame task). 0600B710 runs 0600B524 per frame (sets 2600E51C = -1, puts job 0600B370 in 26016A98, writes SINIT 21000000) or restores the saved context; 0600B8D8 is the main loop. My sat_interrupt loop (64 hops max) is the same as yours.
4. The slave (your pass 4 did the real fix): 06005EE8 is SSHOFF/SSHON with the entry stored at 26000250 (hand-named smpc_SoundOffOn in hand_LORT.txt: that name is wrong, rename it slave_Restart). Slave entry 06005F20; handler 06006000 runs the job then clears FTCSR; job 0600B370 = vdp1_StartCommandTable + 0600CE6C + fx_AllocSpriteRecords (clears 2600E51C) + 060F45D8. Master waits at 0600B5C0 on 26016A70 and in rend_ImportPendingRecords 0600E48A on 2600E51C.

## Results (headless, deterministic: a traced and an untraced run match exactly)
- build\run7/run8 (700 VBlanks): 546 frame changes, 208 draws; shot-700 = Scavenger logo (first picture).
- build\run9 (3000 VBlanks, 50.0 s): 1848 draws, 68000 on, 1332915 non-silent samples; shot-1500 "Creating save file.", shot-2400 title "PRESS START BUTTON". backup.bin written per run dir.
- build\run10 (4200): --input "2500:START,2520:,2900:START,2920:,3300:START,3320:" -> main menu at 2700 (Start Race / Options); START does not select in the menu, C or A does (your script in pass 5 is right).
- build\run11: killed when you renamed the exe; nothing learned.
- build\run6 (2400 VBlanks) once reported only 3 draws; never reproduced, treat as the exe being swapped under it.

## Dead ends / gotchas
- Seeding 0600B710 alone did not fix the task switch; the runtime loop was needed.
- The sh2 CLI takes --at/--count (no --start/--end); --refs only scans the listed window, so scan LORT.BIN bytes with Python for pointer refs instead.
- Two Bash calls that both cd in parallel drift the shared cwd (looked like saturn.exe vanished).
- The old Khronos download was fetched without the user's approval (disclosed to the user).

## Not done (yours now)
- Reach a race and run fullscreen (sh run.sh / run.bat); check VDP2 RBG1 rotation-by-window (your pass 5 note).
- Sound was silent before the 68000 started; after run9 it is on, nothing to do.
- Public repo github.com/indieindie7/scorcher-decomp got native/ (saturnkit-patches.diff of the shared clone at 01:00, build.sh, run.sh, run.bat, README.md) and NOTES.md in commit 5e7f872. Push again only when the user asks; never the emitted C++, extracted data or deps.
