#!/usr/bin/env bash
# Build standalone OpenCL benchmark (no system headers required)
# This version uses dlopen to dynamically load OpenCL at runtime

set -e

NDK_PATH="/opt/android-ndk-r21e"
TOOLCHAIN="$NDK_PATH/toolchains/llvm/prebuilt/linux-x86_64"

# Create output directory
mkdir -p dist

# Common flags
COMMON_FLAGS="-std=c++17 -O3 -Wall -Wno-comment"
STANDALONE_FLAGS="-DLINUX -ldl -pthread"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

print_status() { echo -e "${YELLOW}[BUILD] $1${NC}"; }
print_success() { echo -e "${GREEN}[SUCCESS] $1${NC}"; }
print_error() { echo -e "${RED}[ERROR] $1${NC}"; }

# Build for Linux x86_64
build_linux_x86_64() {
    print_status "Building for Linux x86_64 (standalone)..."
    g++ src/standalone_benchmark.cpp -o dist/OpenCL-Benchmark-standalone-linux-x86_64 \
        $COMMON_FLAGS $STANDALONE_FLAGS
    print_success "Linux x86_64 build completed"
}

# Build for Linux ARM32
build_linux_arm32() {
    print_status "Building for Linux ARM32 (standalone)..."
    if command -v arm-linux-gnueabihf-g++ &> /dev/null; then
        arm-linux-gnueabihf-g++ src/standalone_benchmark.cpp \
            -o dist/OpenCL-Benchmark-standalone-linux-arm32 \
            $COMMON_FLAGS $STANDALONE_FLAGS
        print_success "Linux ARM32 build completed"
    else
        print_error "arm-linux-gnueabihf-g++ not found"
    fi
}

# Build for Linux ARM64
build_linux_arm64() {
    print_status "Building for Linux ARM64 (standalone)..."
    if command -v aarch64-linux-gnu-g++ &> /dev/null; then
        aarch64-linux-gnu-g++ src/standalone_benchmark.cpp \
            -o dist/OpenCL-Benchmark-standalone-linux-arm64 \
            $COMMON_FLAGS $STANDALONE_FLAGS
        print_success "Linux ARM64 build completed"
    else
        print_error "aarch64-linux-gnu-g++ not found"
    fi
}

# Build for Android ARM32
build_android_arm32() {
    print_status "Building for Android ARM32 (standalone)..."
    CC="$TOOLCHAIN/bin/armv7a-linux-androideabi21-clang++"
    if [ -f "$CC" ]; then
        $CC src/standalone_benchmark.cpp -o dist/OpenCL-Benchmark-standalone-android-arm32 \
            $COMMON_FLAGS -static-libstdc++ -ldl
        print_success "Android ARM32 build completed"
    else
        print_error "NDK compiler not found: $CC"
    fi
}

# Build for Android ARM64
build_android_arm64() {
    print_status "Building for Android ARM64 (standalone)..."
    CC="$TOOLCHAIN/bin/aarch64-linux-android21-clang++"
    if [ -f "$CC" ]; then
        $CC src/standalone_benchmark.cpp -o dist/OpenCL-Benchmark-standalone-android-arm64 \
            $COMMON_FLAGS -static-libstdc++ -ldl
        print_success "Android ARM64 build completed"
    else
        print_error "NDK compiler not found: $CC"
    fi
}

echo "=============================================="
echo "  Standalone OpenCL-Benchmark Build"
echo "  (No system OpenCL headers required)"
echo "=============================================="
echo ""

build_linux_x86_64
build_linux_arm32
build_linux_arm64
build_android_arm32
build_android_arm64

echo ""
echo "=============================================="
echo "  Build Summary"
echo "=============================================="
ls -lh dist/OpenCL-Benchmark-standalone-*
echo ""
print_success "All standalone builds completed!"
