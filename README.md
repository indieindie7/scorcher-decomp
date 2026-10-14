# Scorcher (Zyrinx / Scavenger, 1996-97) reverse-engineering notes and symbols

Hand-named function maps, Ghidra scripts, extraction tools and working notes for the
futuristic racing game **Scorcher** (PC DOS and Win95 builds, Watcom x86; Sega Saturn build, SH-2).
Non-commercial, done for study and preservation.

## What is here

| Path | Content |
|---|---|
| `NOTES.md` | the full working log: disc layout, file formats, engine structure, every naming pass |
| `ghidra/hand_scordos.txt` | 2133 hand-named functions of the DOS build (`SCORDOS.EXE`, flattened LE image) with comments |
| `ghidra/hand_SCORWIN.txt` | 716 hand-named functions of the Win95 build (`SCORWIN.EXE`) |
| `ghidra/hand_LORT.txt` | 657 hand lines for the Saturn game program `LORT.BIN` (741 functions, all named) |
| `ghidra/funcinfo_*.tsv` | per-function export: address, name, size, callees, string references |
| `ghidra/xalign_*.txt`, `sig_*.txt`, `restore_*.txt` | cross-build alignment and Watcom runtime signature lists |
| `ghidra/globals_*.txt` | global variable labels (`0x<addr> <name>`, apply with `scripts/ApplyGlobals.java`): DOS 1126, Win95 832, Saturn 475 variables + 1363 `p_<name>` literal-pool slots; names with an address suffix are best guesses. The naming tools are in the amok-decomp repo (`tools/`) |
| `ghidra/types_*.txt` | function signatures (parameter and return types) |
| `ghidra/structtypes_*.txt` | globals typed with the `MAINENG.ASC` structs (import them with `scripts/ImportMainengStructs.java`, then apply with `ApplyGlobals.java`) |
| `ghidra/x86watcom.cspec` | compiler spec for the Watcom register calling convention |
| `ghidra/scripts/*.java` | headless Ghidra scripts: find function starts, repair bodies, apply names, export, dump decompilation |
| `ghidra/decomp/*.c` | decompiler dumps of selected routines (routine table, wl3 loaders) |
| `pcwin/` | SCORWIN on Windows 11: our own `ddraw.dll` proxy (`ddraw/ddraw.c`) with options overlay, CD music from WAV, and the paused Manx TT bike import (`MANX-IMPORT.md`: what was built, collider findings, open bugs) |
| `native/` | native build work (`HANDOFF-native.md`); `game/manx_bike.h` = Manx TT bike model reimplemented from the manxtt-decomp study |
| `physics_compare.md`, `bike_physics_papers.md` | handling comparison of Scorcher / Manx TT / other racers, and bike dynamics papers |
| `tools/*.py` | ISO / MEG / wl3 extractors, LE flattener, Watcom OMF signature builder, cross-build aligner |

Line format of the hand files: `0x<addr> func <name> hand <comment>`. The comment describes the body,
callers, data addresses and strings as read by hand. Names with `_sh2` are C runtime copies on Saturn,
`_asm` are register-only assembly helpers, `stub_<addr>` are unread stubs.

## What is NOT here

No game files. No executables, disc images, `LORT.BIN`, level data, textures, music, or the
Ghidra project database (which embeds the binaries). You need your own copy of the game.

## Reproducing the Ghidra project

1. Ghidra 12.1.4 (headless). Create a project named `Scorcher`.
2. Import `LORT.BIN` (SH-2, base 0x06004000), `SCORWIN.EXE` (PE) and the flattened DOS image
   made by `tools/leflatten.py` from `SCORDOS.EXE` (DOS/4GW LE). `NOTES.md` has the exact steps,
   the Watcom cspec install and the function-start repair order.
3. Run `ghidra/scripts/ApplyNames.java` with the matching `hand_<program>.txt`, then
   `ExportFuncInfo.java` to regenerate the tsv.

## Rights and takedown

Scorcher is copyright its rights holders (Zyrinx / Scavenger, Inc., 1996-97; the publisher went
bankrupt in 1998 and the current holder is unknown). This repository contains only work made by
reading the shipped program: symbol names, comments, notes, scripts and tools. The symbol names,
comments and decompiler dumps are offered without any licence claim of our own over the game.
The scripts and tools (`ghidra/scripts`, `tools`, `x86watcom.cspec`) are MIT licensed (see `LICENSE`).

**If you hold the rights and want this removed, file a notice through GitHub's DMCA form.
It will not be contested.** No need to contact the author first.
