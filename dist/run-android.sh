#!/bin/sh
# OpenCL-Benchmark Smart Launcher for Android
# Automatically detects and configures Mali GPU library paths

# Detect architecture
ARCH=$(uname -m)
case "$ARCH" in
    armv7*|armhf)
        ARCH_TYPE="arm32"
        LIB_PATHS="/system/lib /system/vendor/lib /vendor/lib /vendor/lib/egl /vendor/lib/hw"
        ;;
    aarch64|arm64)
        ARCH_TYPE="arm64"
        LIB_PATHS="/system/lib64 /system/vendor/lib64 /vendor/lib64 /vendor/lib64/egl /vendor/lib64/hw"
        ;;
    *)
        echo "Unsupported architecture: $ARCH"
        exit 1
        ;;
esac

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
EXECUTABLE="$SCRIPT_DIR/OpenCL-Benchmark-android-$ARCH_TYPE"

if [ ! -f "$EXECUTABLE" ]; then
    echo "Error: Executable not found: $EXECUTABLE"
    exit 1
fi

# Build library path
LD_LIB_PATH=""

# Check for Mali GPU library locations
for dir in $LIB_PATHS; do
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
echo "  OpenCL-Benchmark Launcher (Android)"
echo "=============================================="
echo "Architecture: $ARCH ($ARCH_TYPE)"
echo "Library Path: $LD_LIB_PATH"
echo "=============================================="

# Export and run
export LD_LIBRARY_PATH="$LD_LIB_PATH"
exec "$EXECUTABLE" "$@"
