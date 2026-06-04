#!/bin/sh
# OpenCL-Benchmark Launcher for Android ARM32
# This script sets up the library path and runs the benchmark

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
EXECUTABLE="$SCRIPT_DIR/OpenCL-Benchmark-android-arm32"

# Android library paths for Mali GPUs
# Common locations: /vendor/lib/egl/libGLES_mali.so
export LD_LIBRARY_PATH=/system/lib:/system/vendor/lib:/vendor/lib:/vendor/lib/egl:$SCRIPT_DIR/lib:$LD_LIBRARY_PATH

exec "$EXECUTABLE" "$@"
