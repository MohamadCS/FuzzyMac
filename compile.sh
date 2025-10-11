#!/usr/bin/env bash
set -euo pipefail

# Configuration
BUILD_DIR="build"
APP_NAME="FuzzyMac"

# Build
cmake -S . -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build "$BUILD_DIR" -j$(sysctl -n hw.ncpu)

# Kill old instance if running
pkill -x "$APP_NAME" 2>/dev/null || true

