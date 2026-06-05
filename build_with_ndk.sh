#!/usr/bin/env bash
# Build script using Android NDK for proper TLS alignment
# This fixes the "executable's TLS segment is underaligned" error

set -e

NDK_PATH="/opt/android-ndk-r21e"
TOOLCHAIN="$NDK_PATH/toolchains/llvm/prebuilt/linux-x86_64"

# Create output directory
mkdir -p dist

# Source files
SOURCES="src/main.cpp src/kernel.cpp src/opencl_stub.cpp"

# Common compile flags
COMMON_FLAGS="-std=c++17 -O3 -Wno-comment -I./src/OpenCL/include"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

print_status() { echo -e "${YELLOW}[BUILD] $1${NC}"; }
print_success() { echo -e "${GREEN}[SUCCESS] $1${NC}"; }
print_error() { echo -e "${RED}[ERROR] $1${NC}"; }

# Build for Android ARM32 using NDK
build_android_arm32() {
    print_status "Building for Android ARM32 using NDK..."
    
    # NDK compiler for ARM32 (API level 21 for broad compatibility)
    CC="$TOOLCHAIN/bin/armv7a-linux-androideabi21-clang++"
    
    if [ -f "$CC" ]; then
        $CC $SOURCES -o dist/OpenCL-Benchmark-android-arm32 \
            $COMMON_FLAGS \
            -static-libstdc++ -ldl \
            -Wl,-rpath,'/system/lib:/vendor/lib:/vendor/lib/egl'
        print_success "Android ARM32 build completed"
    else
        print_error "NDK compiler not found: $CC"
    fi
}

# Build for Android ARM64 using NDK
build_android_arm64() {
    print_status "Building for Android ARM64 using NDK..."
    
    # NDK compiler for ARM64 (API level 21 for broad compatibility)
    CC="$TOOLCHAIN/bin/aarch64-linux-android21-clang++"
    
    if [ -f "$CC" ]; then
        $CC $SOURCES -o dist/OpenCL-Benchmark-android-arm64 \
            $COMMON_FLAGS \
            -static-libstdc++ -ldl \
            -Wl,-rpath,'/system/lib64:/vendor/lib64:/vendor/lib64/egl'
        print_success "Android ARM64 build completed"
    else
        print_error "NDK compiler not found: $CC"
    fi
}

# Build for Linux ARM32 (keep using cross compiler)
build_linux_arm32() {
    print_status "Building for Linux ARM32..."
    if command -v arm-linux-gnueabihf-g++ &> /dev/null; then
        arm-linux-gnueabihf-g++ $SOURCES -o dist/OpenCL-Benchmark-linux-arm32 \
            $COMMON_FLAGS \
            -pthread -ldl \
            -Wl,-rpath,'/lib:/lib64:/usr/lib:/usr/lib64'
        print_success "Linux ARM32 build completed"
    else
        print_error "arm-linux-gnueabihf-g++ not found"
    fi
}

# Build for Linux ARM64 (keep using cross compiler)
build_linux_arm64() {
    print_status "Building for Linux ARM64..."
    if command -v aarch64-linux-gnu-g++ &> /dev/null; then
        aarch64-linux-gnu-g++ $SOURCES -o dist/OpenCL-Benchmark-linux-arm64 \
            $COMMON_FLAGS \
            -pthread -ldl \
            -Wl,-rpath,'/lib:/lib64:/usr/lib:/usr/lib64'
        print_success "Linux ARM64 build completed"
    else
        print_error "aarch64-linux-gnu-g++ not found"
    fi
}

echo "=============================================="
echo "  OpenCL-Benchmark Build with Android NDK"
echo "  NDK: $NDK_PATH"
echo "=============================================="
echo ""

build_linux_arm32
build_linux_arm64
build_android_arm32
build_android_arm64

echo ""
echo "=============================================="
echo "  Build Summary"
echo "=============================================="
ls -la dist/OpenCL-Benchmark-*
echo ""
print_success "Build completed!"
