#!/system/bin/sh
# OpenCL-Benchmark Android Launcher
# Automatically finds and loads OpenCL/Mali GPU libraries

# Determine architecture
ARCH=$(uname -m)
case "$ARCH" in
    aarch64|arm64)
        BINARY="OpenCL-Benchmark-android-arm64"
        LIB_DIRS="/system/lib64 /vendor/lib64 /vendor/lib64/egl /vendor/lib64/hw"
        ;;
    armv7*|armhf)
        BINARY="OpenCL-Benchmark-android-arm32"
        LIB_DIRS="/system/lib /vendor/lib /vendor/lib/egl /vendor/lib/hw"
        ;;
    *)
        echo "Unsupported architecture: $ARCH"
        exit 1
        ;;
esac

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
EXECUTABLE="$SCRIPT_DIR/$BINARY"

if [ ! -f "$EXECUTABLE" ]; then
    echo "Error: Executable not found: $EXECUTABLE"
    exit 1
fi

echo "=============================================="
echo "  OpenCL-Benchmark Android Launcher"
echo "=============================================="
echo "Architecture: $ARCH"
echo "Executable: $BINARY"
echo ""

# Function to check if a library exists
check_lib() {
    local dir="$1"
    [ -f "$dir/libOpenCL.so" ] || [ -f "$dir/libOpenCL.so.1" ] || \
    [ -f "$dir/libmali.so" ] || [ -f "$dir/libMali.so" ] || \
    [ -f "$dir/egl/libGLES_mali.so" ] || \
    [ -f "$dir/libGLES_mali.so" ]
}

# Search for OpenCL/Mali libraries
echo "Searching for OpenCL/Mali GPU libraries..."
FOUND_LIBS=""

for dir in $LIB_DIRS; do
    if [ -d "$dir" ]; then
        # Check for various library names
        for lib in libOpenCL.so libOpenCL.so.1 libmali.so libMali.so libGLES_mali.so; do
            if [ -f "$dir/$lib" ]; then
                echo "  Found: $dir/$lib"
                FOUND_LIBS="$FOUND_LIBS:$dir"
                break 2
            fi
        done
        # Check egl subdirectory
        if [ -d "$dir/egl" ]; then
            if [ -f "$dir/egl/libGLES_mali.so" ]; then
                echo "  Found: $dir/egl/libGLES_mali.so"
                FOUND_LIBS="$FOUND_LIBS:$dir:$dir/egl"
                break
            fi
        fi
    fi
done

# Also check common Mali GPU paths
for path in /vendor/lib*/egl/libGLES_mali.so /vendor/lib*/libmali.so; do
    if [ -f "$path" ]; then
        libdir=$(dirname "$path")
        echo "  Found: $path"
        FOUND_LIBS="$FOUND_LIBS:$libdir"
    fi
done

if [ -z "$FOUND_LIBS" ]; then
    echo ""
    echo "WARNING: No OpenCL/Mali libraries found!"
    echo ""
    echo "Please check if your device has:"
    echo "  1. Mali GPU driver installed"
    echo "  2. OpenCL support enabled"
    echo ""
    echo "Common library locations:"
    echo "  /vendor/lib64/egl/libGLES_mali.so"
    echo "  /vendor/lib64/libmali.so"
    echo "  /vendor/lib/egl/libGLES_mali.so"
    echo "  /vendor/lib/libmali.so"
    echo ""
    echo "You can manually set LD_LIBRARY_PATH and run:"
    echo "  export LD_LIBRARY_PATH=/vendor/lib64/egl:\$LD_LIBRARY_PATH"
    echo "  $EXECUTABLE"
    echo ""
fi

# Build library path
LD_LIB_PATH=""
for dir in $LIB_DIRS; do
    if [ -d "$dir" ]; then
        if [ -z "$LD_LIB_PATH" ]; then
            LD_LIB_PATH="$dir"
        else
            LD_LIB_PATH="$LD_LIB_PATH:$dir"
        fi
    fi
done

# Add local lib directory if exists
if [ -d "$SCRIPT_DIR/lib" ]; then
    LD_LIB_PATH="$LD_LIB_PATH:$SCRIPT_DIR/lib"
fi

echo ""
echo "Library path: $LD_LIB_PATH"
echo "=============================================="
echo ""

# Export library path and run
export LD_LIBRARY_PATH="$LD_LIB_PATH"

# Try to preload Mali library if found
MALI_LIB=""
for dir in $LIB_DIRS; do
    if [ -f "$dir/libGLES_mali.so" ]; then
        MALI_LIB="$dir/libGLES_mali.so"
        break
    fi
    if [ -f "$dir/egl/libGLES_mali.so" ]; then
        MALI_LIB="$dir/egl/libGLES_mali.so"
        break
    fi
done

if [ -n "$MALI_LIB" ]; then
    echo "Preloading Mali library: $MALI_LIB"
    export LD_PRELOAD="$MALI_LIB"
fi

echo "Running benchmark..."
echo ""
exec "$EXECUTABLE" "$@"
