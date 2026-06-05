#!/usr/bin/env bash
# Build script for Android ARM with proper TLS alignment
# Fixes: "executable's TLS segment is underaligned" error

set -e

# Create output directory
mkdir -p dist

# Source files
STUB_SOURCES="src/main.cpp src/kernel.cpp src/opencl_stub.cpp"

# Common compile flags
COMMON_FLAGS="-std=c++17 -O3 -Wno-comment -I./src/OpenCL/include"

# TLS alignment fix flags for Android
ANDROID_FLAGS="-ftls-model=initial-exec"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

print_status() { echo -e "${YELLOW}[BUILD] $1${NC}"; }
print_success() { echo -e "${GREEN}[SUCCESS] $1${NC}"; }
print_error() { echo -e "${RED}[ERROR] $1${NC}"; }

# Build for Android ARM32
build_android_arm32() {
    print_status "Building for Android ARM32 with TLS fix..."
    if command -v arm-linux-gnueabihf-g++ &> /dev/null; then
        arm-linux-gnueabihf-g++ $STUB_SOURCES -o dist/OpenCL-Benchmark-android-arm32 \
            $COMMON_FLAGS $ANDROID_FLAGS \
            -pthread -ldl \
            -static-libgcc -static-libstdc++ \
            -Wl,--no-warn-mismatch
        print_success "Android ARM32 build completed"
    else
        print_error "arm-linux-gnueabihf-g++ not found"
    fi
}

# Build for Android ARM64 with TLS alignment fix
build_android_arm64() {
    print_status "Building for Android ARM64 with TLS fix..."
    if command -v aarch64-linux-gnu-g++ &> /dev/null; then
        aarch64-linux-gnu-g++ $STUB_SOURCES -o dist/OpenCL-Benchmark-android-arm64 \
            $COMMON_FLAGS $ANDROID_FLAGS \
            -pthread -ldl \
            -static-libgcc -static-libstdc++ \
            -Wl,--no-warn-mismatch \
            -fPIC
        print_success "Android ARM64 build completed"
    else
        print_error "aarch64-linux-gnu-g++ not found"
    fi
}

# Build for Linux ARM32
build_linux_arm32() {
    print_status "Building for Linux ARM32..."
    if command -v arm-linux-gnueabihf-g++ &> /dev/null; then
        arm-linux-gnueabihf-g++ $STUB_SOURCES -o dist/OpenCL-Benchmark-linux-arm32 \
            $COMMON_FLAGS \
            -pthread -ldl \
            -Wl,-rpath,'/lib:/lib64:/usr/lib:/usr/lib64'
        print_success "Linux ARM32 build completed"
    else
        print_error "arm-linux-gnueabihf-g++ not found"
    fi
}

# Build for Linux ARM64
build_linux_arm64() {
    print_status "Building for Linux ARM64..."
    if command -v aarch64-linux-gnu-g++ &> /dev/null; then
        aarch64-linux-gnu-g++ $STUB_SOURCES -o dist/OpenCL-Benchmark-linux-arm64 \
            $COMMON_FLAGS \
            -pthread -ldl \
            -Wl,-rpath,'/lib:/lib64:/usr/lib:/usr/lib64'
        print_success "Linux ARM64 build completed"
    else
        print_error "aarch64-linux-gnu-g++ not found"
    fi
}

echo "=============================================="
echo "  OpenCL-Benchmark ARM Build"
echo "  With TLS Alignment Fix for Android"
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
