@echo off
rem Scorcher, native (saturnkit recompilation), borderless fullscreen.
rem Keys: arrows, Enter = Start, Z/X/C = A/B/C, A/S/D = X/Y/Z, Q/W = L/R.
rem F11 or Alt+Enter toggles fullscreen, F12 saves a screenshot, closing the window quits.
setlocal
set PATH=C:\msys64\mingw64\bin;%PATH%
cd /d "%~dp0build"
if not exist play mkdir play
if not exist play\backup.bin copy run4\backup.bin play\ >nul
recomp-build\saturn.exe --cue "%~dp0..\saturn\image\Scorcher.cue" --fullscreen --out play %*
