#!/bin/sh
# Run Scorcher natively, borderless fullscreen. Extra args go to saturn (e.g. --scale 3, --headless).
HERE=$(cd "$(dirname "$0")" && pwd)
exec "$HERE/build/recomp-build/saturn.exe" --cue "C:/Users/john/Documents/Scorcher/saturn/image/Scorcher.cue" --fullscreen "$@"
