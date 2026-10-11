# Native Windows build of Saturn Scorcher (via saturnkit)

Static recompilation of the Saturn build (SH-2 machine code to C++) with the
saturnkit runtime (SDL3). No game data is in this repo: you need your own
Scorcher disc image (.cue/.bin).

Status (2026-10-11): boots through the logos, save creation, title screen and
main menu with sound; the modding session also reached the PREPARE TO RACE
screen. Not yet playable. Details and exact addresses: NOTES.md, the
"Native build" entries (step 1-8 and pass 1-5).

## Reproduce

1. Clone saturnkit (vs-sr-dev, MIT) at commit e4a6fbd into native/saturnkit and
   apply native/saturnkit-patches.diff (git apply). The patches: VDP1 MODR
   mirror, interrupt dispatcher follows an rte into another task, FRT
   input-capture interrupt for the slave SH-2, VDP2 zoom defaults, a
   --landing recomp option.
2. Extract the disc:  py -3.13 -m saturnkit.disc Scorcher.cue --extract build/extract
3. Emit the C++:
   py -3.13 -m saturnkit.recomp --out build/recomp --overlay CDCON --landing LORT:0600D0C8 ZERO=build/extract/0@06010000 LORT=build/extract/LORT.BIN@06004000+0600B710 INTRO=build/extract/INTRO.BIN@00280000 CDCON=build/extract/CDCON.BIN@00200000
4. Toolchain: LLVM clang (winget install LLVM.LLVM), VS 2026 cmake + ninja,
   SDL3 3.4.18 VC devel zip unpacked to deps/sdl3-dl/SDL3-3.4.18, Khronos
   GL/glcorearb.h + KHR/khrplatform.h in deps/khronos. Then: sh build.sh
   (Git Bash) or the equivalent cmake/ninja commands inside it.
5. Run: run.bat (double-click; edit the CUE path inside) or sh run.sh.
   Headless test: saturn.exe --cue X.cue --headless --vblanks 3000 --out run --shot 2400
   Pad script: --input "2500:START,2520:,2900:C,2920:"
