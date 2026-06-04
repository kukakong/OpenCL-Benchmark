#!/bin/bash
# OpenCL-Benchmark Launcher for Linux ARM32
# This script sets up the library path and runs the benchmark

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EXECUTABLE="$SCRIPT_DIR/OpenCL-Benchmark-linux-arm32"

# Common library paths for Mali GPUs on Linux ARM
LIB_PATHS=(
    "/lib"
    "/lib64"
    "/usr/lib"
    "/usr/lib64"
    "/usr/local/lib"
    "/usr/local/lib64"
    "$SCRIPT_DIR/lib"
)

# Build LD_LIBRARY_PATH
LD_LIB_PATH=""
for path in "${LIB_PATHS[@]}"; do
    if [ -d "$path" ]; then
        if [ -z "$LD_LIB_PATH" ]; then
            LD_LIB_PATH="$path"
        else
            LD_LIB_PATH="$LD_LIB_PATH:$path"
        fi
    fi
done

# Export and run
export LD_LIBRARY_PATH="$LD_LIB_PATH${LD_LIBRARY_PATH:+:}$LD_LIBRARY_PATH"
exec "$EXECUTABLE" "$@"
