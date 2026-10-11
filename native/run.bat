@echo off
rem Runs the native Saturn Scorcher build borderless fullscreen. No Git Bash needed.
rem Edit CUE if your disc image lives elsewhere. Needs build\recomp-build\saturn.exe (see README.md).
set CUE=C:\Users\john\Documents\Scorcher\saturn\image\Scorcher.cue
cd /d "%~dp0"
build\recomp-build\saturn.exe --cue "%CUE%" --fullscreen %*
