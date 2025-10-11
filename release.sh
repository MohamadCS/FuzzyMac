#!/usr/bin/env bash
set -euo pipefail

# Configuration
BUILD_DIR="release"
APP_NAME="FuzzyMac"

# Build
cmake -S . -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=OFF
cmake --build "$BUILD_DIR" -j$(sysctl -n hw.ncpu)

# Kill any running instance
pkill -x "$APP_NAME" 2>/dev/null || true

