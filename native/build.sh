#!/bin/sh
# Scorcher native build (saturnkit). Run from Documents/Scorcher/native in Git Bash.
# Needs: LLVM (clang++), VS 18 cmake+ninja, SDL3 VC package in deps/sdl3-dl, Khronos GL headers in deps/khronos.
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
CM="/c/Program Files/Microsoft Visual Studio/18/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe"
NJ="/c/Program Files/Microsoft Visual Studio/18/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe"
export PATH="/c/Program Files/LLVM/bin:$(dirname "$NJ"):$PATH"
SDL3_DIR="$HERE/deps/sdl3-dl/SDL3-3.4.18/cmake"
"$CM" -S "$HERE/build/recomp" -B "$HERE/build/recomp-build" -G Ninja \
  -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang \
  -DCMAKE_BUILD_TYPE=Release \
  -DSDL3_DIR="$SDL3_DIR" \
  -DSATURNKIT_EXTRA="$HERE/game/scorcher.cmake" \
  -DCMAKE_CXX_FLAGS="-I$HERE/deps/khronos -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS" \
  -DCMAKE_C_FLAGS="-I$HERE/deps/khronos"
"$NJ" -C "$HERE/build/recomp-build" "$@"
cp -n "$HERE/deps/sdl3-dl/SDL3-3.4.18/lib/x64/SDL3.dll" "$HERE/build/recomp-build/" 2>/dev/null || true
echo "built: $HERE/build/recomp-build/saturn.exe"
