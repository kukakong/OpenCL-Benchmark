#!/bin/sh
# OpenCL-Benchmark Launcher for Android ARM64
# This script sets up the library path and runs the benchmark

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
EXECUTABLE="$SCRIPT_DIR/OpenCL-Benchmark-android-arm64"

# Android library paths for Mali GPUs
# Common locations: /vendor/lib64/egl/libGLES_mali.so
export LD_LIBRARY_PATH=/system/lib64:/system/vendor/lib64:/vendor/lib64:/vendor/lib64/egl:$SCRIPT_DIR/lib:$LD_LIBRARY_PATH

exec "$EXECUTABLE" "$@"
