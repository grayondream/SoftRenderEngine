#!/usr/bin/env bash
set -e
cd "$(dirname "$0")/.."

BUILD_DIR=build
if [ ! -f "$BUILD_DIR/CMakeCache.txt" ]; then
    echo "[buildandrun] configuring..."
    cmake -B "$BUILD_DIR" -S .
fi

echo "[buildandrun] building..."
cmake --build "$BUILD_DIR" -j"$(nproc)"

BIN="$BUILD_DIR/src/soft-game-engine"
if [ ! -x "$BIN" ]; then
    echo "[buildandrun] binary not found: $BIN"
    exit 1
fi

echo "[buildandrun] running $BIN $*"
exec "$BIN" "$@"
