#!/usr/bin/env bash
# Cross-platform build script for OpenCL-Benchmark
# Supports: Linux ARM32, Linux ARM64, Android ARM32, Android ARM64
# With dynamic library loading support for Mali GPUs

set -e

# Create output directory
mkdir -p dist

# Source files
STUB_SOURCES="src/main.cpp src/kernel.cpp src/opencl_stub.cpp"

# Common compile flags
COMMON_FLAGS="-std=c++17 -O3 -Wno-comment -I./src/OpenCL/include"

# For dynamic linking, use -Wl,-rpath to embed library search paths
# This allows the executable to find libraries at runtime

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${YELLOW}[BUILD] $1${NC}"
}

print_success() {
    echo -e "${GREEN}[SUCCESS] $1${NC}"
}

print_error() {
    echo -e "${RED}[ERROR] $1${NC}"
}

# 1. Build for Linux ARM32 (armhf) with dynamic library support
build_linux_arm32() {
    print_status "Building for Linux ARM32 (armhf) with dynamic library support..."
    if command -v arm-linux-gnueabihf-g++ &> /dev/null; then
        # Compile with rpath for common library locations
        arm-linux-gnueabihf-g++ $STUB_SOURCES -o dist/OpenCL-Benchmark-linux-arm32 $COMMON_FLAGS \
            -pthread -ldl \
            -Wl,-rpath,'/lib:/lib64:/usr/lib:/usr/lib64:/usr/local/lib:/usr/local/lib64' \
            -Wl,-rpath,'$ORIGIN/lib' \
            -Wl,--allow-shlib-undefined
        print_success "Linux ARM32 build completed"
    else
        print_error "arm-linux-gnueabihf-g++ not found, skipping Linux ARM32 build"
    fi
}

# 2. Build for Linux ARM64 with dynamic library support
build_linux_arm64() {
    print_status "Building for Linux ARM64 with dynamic library support..."
    if command -v aarch64-linux-gnu-g++ &> /dev/null; then
        # Compile with rpath for common library locations
        aarch64-linux-gnu-g++ $STUB_SOURCES -o dist/OpenCL-Benchmark-linux-arm64 $COMMON_FLAGS \
            -pthread -ldl \
            -Wl,-rpath,'/lib:/lib64:/usr/lib:/usr/lib64:/usr/local/lib:/usr/local/lib64' \
            -Wl,-rpath,'$ORIGIN/lib' \
            -Wl,--allow-shlib-undefined
        print_success "Linux ARM64 build completed"
    else
        print_error "aarch64-linux-gnu-g++ not found, skipping Linux ARM64 build"
    fi
}

# 3. Build for Android ARM32 with dynamic library support
build_android_arm32() {
    print_status "Building for Android ARM32 with dynamic library support..."
    if command -v arm-linux-gnueabihf-g++ &> /dev/null; then
        # Android specific: link against system libraries
        arm-linux-gnueabihf-g++ $STUB_SOURCES -o dist/OpenCL-Benchmark-android-arm32 $COMMON_FLAGS \
            -pthread -ldl -landroid \
            -Wl,-rpath,'/system/lib:/system/vendor/lib:/vendor/lib:/vendor/lib/egl' \
            -Wl,-rpath,'$ORIGIN/lib' \
            -Wl,--allow-shlib-undefined
        print_success "Android ARM32 build completed"
    else
        print_error "arm-linux-gnueabihf-g++ not found, skipping Android ARM32 build"
    fi
}

# 4. Build for Android ARM64 with dynamic library support
build_android_arm64() {
    print_status "Building for Android ARM64 with dynamic library support..."
    if command -v aarch64-linux-gnu-g++ &> /dev/null; then
        # Android specific: link against system libraries
        aarch64-linux-gnu-g++ $STUB_SOURCES -o dist/OpenCL-Benchmark-android-arm64 $COMMON_FLAGS \
            -pthread -ldl -landroid \
            -Wl,-rpath,'/system/lib64:/system/vendor/lib64:/vendor/lib64:/vendor/lib64/egl' \
            -Wl,-rpath,'$ORIGIN/lib' \
            -Wl,--allow-shlib-undefined
        print_success "Android ARM64 build completed"
    else
        print_error "aarch64-linux-gnu-g++ not found, skipping Android ARM64 build"
    fi
}

# Create launcher scripts for Linux
create_linux_launcher() {
    print_status "Creating launcher scripts for Linux..."
    
    # ARM32 launcher
    cat > dist/run-linux-arm32.sh << 'LAUNCHER'
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
LAUNCHER
    chmod +x dist/run-linux-arm32.sh
    
    # ARM64 launcher
    cat > dist/run-linux-arm64.sh << 'LAUNCHER'
#!/bin/bash
# OpenCL-Benchmark Launcher for Linux ARM64
# This script sets up the library path and runs the benchmark

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EXECUTABLE="$SCRIPT_DIR/OpenCL-Benchmark-linux-arm64"

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
LAUNCHER
    chmod +x dist/run-linux-arm64.sh
    
    print_success "Linux launcher scripts created"
}

# Create launcher scripts for Android
create_android_launcher() {
    print_status "Creating launcher scripts for Android..."
    
    # ARM32 launcher
    cat > dist/run-android-arm32.sh << 'LAUNCHER'
#!/bin/sh
# OpenCL-Benchmark Launcher for Android ARM32
# This script sets up the library path and runs the benchmark

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
EXECUTABLE="$SCRIPT_DIR/OpenCL-Benchmark-android-arm32"

# Android library paths for Mali GPUs
# Common locations: /vendor/lib/egl/libGLES_mali.so
export LD_LIBRARY_PATH=/system/lib:/system/vendor/lib:/vendor/lib:/vendor/lib/egl:$SCRIPT_DIR/lib:$LD_LIBRARY_PATH

exec "$EXECUTABLE" "$@"
LAUNCHER
    chmod +x dist/run-android-arm32.sh
    
    # ARM64 launcher
    cat > dist/run-android-arm64.sh << 'LAUNCHER'
#!/bin/sh
# OpenCL-Benchmark Launcher for Android ARM64
# This script sets up the library path and runs the benchmark

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
EXECUTABLE="$SCRIPT_DIR/OpenCL-Benchmark-android-arm64"

# Android library paths for Mali GPUs
# Common locations: /vendor/lib64/egl/libGLES_mali.so
export LD_LIBRARY_PATH=/system/lib64:/system/vendor/lib64:/vendor/lib64:/vendor/lib64/egl:$SCRIPT_DIR/lib:$LD_LIBRARY_PATH

exec "$EXECUTABLE" "$@"
LAUNCHER
    chmod +x dist/run-android-arm64.sh
    
    print_success "Android launcher scripts created"
}

# Main build process
echo "=============================================="
echo "  OpenCL-Benchmark Cross-Platform Build"
echo "  With Dynamic Library Support"
echo "=============================================="
echo ""

# Build all targets
build_linux_arm32
build_linux_arm64
build_android_arm32
build_android_arm64

# Create launcher scripts
create_linux_launcher
create_android_launcher

echo ""
echo "=============================================="
echo "  Build Summary"
echo "=============================================="
ls -la dist/
echo ""
print_success "All builds completed successfully!"
echo ""
echo "Usage:"
echo "  Linux:   ./run-linux-arm64.sh [device_ids...]"
echo "  Android: ./run-android-arm64.sh [device_ids...]"
