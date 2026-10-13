#!/bin/sh
# Build from Git Bash / any POSIX shell: ./build.sh [make args]
# 32-bit MinGW for the game, msys make; pret's host tools are 64-bit and find
# their own DLLs via the mingw64 entry later on PATH.
cd "$(dirname "$0")"
mkdir -p build/tmp; export TMP="$(pwd -W)/build/tmp" TEMP="$(pwd -W)/build/tmp" TMPDIR="$(pwd -W)/build/tmp"
export PATH="/c/msys64/mingw32/bin:/c/msys64/usr/bin:$PATH:/c/msys64/mingw64/bin"
exec /c/msys64/usr/bin/make "$@"
