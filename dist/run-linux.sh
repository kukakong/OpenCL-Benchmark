#!/bin/bash
# OpenCL-Benchmark Smart Launcher for Linux ARM32/ARM64
# Automatically detects and configures Mali GPU library paths

set -e

# Detect architecture
ARCH=$(uname -m)
case "$ARCH" in
    armv7*|armhf)
        ARCH_TYPE="arm32"
        LIB_DIRS="/lib /usr/lib /usr/local/lib"
        LIB64_DIRS=""
        ;;
    aarch64|arm64)
        ARCH_TYPE="arm64"
        LIB_DIRS="/lib /usr/lib /usr/local/lib"
        LIB64_DIRS="/lib64 /usr/lib64 /usr/local/lib64"
        ;;
    *)
        echo "Unsupported architecture: $ARCH"
        exit 1
        ;;
esac

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EXECUTABLE="$SCRIPT_DIR/OpenCL-Benchmark-linux-$ARCH_TYPE"

if [ ! -f "$EXECUTABLE" ]; then
    echo "Error: Executable not found: $EXECUTABLE"
    exit 1
fi

# Function to check if library exists
check_lib() {
    local dir="$1"
    [ -f "$dir/libmali.so" ] || [ -f "$dir/libMali.so" ] || \
    [ -f "$dir/libOpenCL.so" ] || [ -f "$dir/libOpenCL.so.1" ]
}

# Build library path
LD_LIB_PATH=""

# Check common Mali GPU library locations
MALI_PATHS=""
for dir in $LIB_DIRS $LIB64_DIRS; do
    if [ -d "$dir" ] && check_lib "$dir"; then
        MALI_PATHS="$MALI_PATHS:$dir"
    fi
done

# Add all library directories to path
for dir in $LIB_DIRS $LIB64_DIRS; do
    if [ -d "$dir" ]; then
        if [ -z "$LD_LIB_PATH" ]; then
            LD_LIB_PATH="$dir"
        else
            LD_LIB_PATH="$LD_LIB_PATH:$dir"
        fi
    fi
done

# Add script directory lib folder
if [ -d "$SCRIPT_DIR/lib" ]; then
    LD_LIB_PATH="$LD_LIB_PATH:$SCRIPT_DIR/lib"
fi

# Print detected information
echo "=============================================="
echo "  OpenCL-Benchmark Launcher"
echo "=============================================="
echo "Architecture: $ARCH ($ARCH_TYPE)"
echo "Library Path: $LD_LIB_PATH"
echo "=============================================="

# Export and run
export LD_LIBRARY_PATH="$LD_LIB_PATH${LD_LIBRARY_PATH:+:}$LD_LIBRARY_PATH"
exec "$EXECUTABLE" "$@"
