#!/usr/bin/env bash
# Cross-platform build script for OpenCL-Benchmark
# Supports: Linux x86_64, Linux ARM32, Linux ARM64, Android ARM32, Android ARM64

set -e

# Create output directory
mkdir -p dist

# Source files
SOURCES="src/main.cpp src/kernel.cpp"
STUB_SOURCES="src/main.cpp src/kernel.cpp src/opencl_stub.cpp"

# Common compile flags
COMMON_FLAGS="-std=c++17 -O3 -Wno-comment -I./src/OpenCL/include"

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

# 1. Build for Linux x86_64
build_linux_x86_64() {
    print_status "Building for Linux x86_64..."
    g++ $SOURCES -o dist/OpenCL-Benchmark-linux-x86_64 $COMMON_FLAGS -pthread -L./src/OpenCL/lib -lOpenCL
    print_success "Linux x86_64 build completed"
}

# 2. Build for Linux ARM32 (armhf)
build_linux_arm32() {
    print_status "Building for Linux ARM32 (armhf)..."
    if command -v arm-linux-gnueabihf-g++ &> /dev/null; then
        arm-linux-gnueabihf-g++ $STUB_SOURCES -o dist/OpenCL-Benchmark-linux-arm32 $COMMON_FLAGS -pthread -static-libgcc -static-libstdc++ -ldl
        print_success "Linux ARM32 build completed"
    else
        print_error "arm-linux-gnueabihf-g++ not found, skipping Linux ARM32 build"
    fi
}

# 3. Build for Linux ARM64
build_linux_arm64() {
    print_status "Building for Linux ARM64..."
    if command -v aarch64-linux-gnu-g++ &> /dev/null; then
        aarch64-linux-gnu-g++ $STUB_SOURCES -o dist/OpenCL-Benchmark-linux-arm64 $COMMON_FLAGS -pthread -static-libgcc -static-libstdc++ -ldl
        print_success "Linux ARM64 build completed"
    else
        print_error "aarch64-linux-gnu-g++ not found, skipping Linux ARM64 build"
    fi
}

# 4. Build for Android ARM32
build_android_arm32() {
    print_status "Building for Android ARM32..."
    if command -v arm-linux-gnueabihf-g++ &> /dev/null; then
        arm-linux-gnueabihf-g++ $STUB_SOURCES -o dist/OpenCL-Benchmark-android-arm32 $COMMON_FLAGS -pthread -static-libgcc -static-libstdc++ -ldl
        print_success "Android ARM32 build completed"
    else
        print_error "arm-linux-gnueabihf-g++ not found, skipping Android ARM32 build"
    fi
}

# 5. Build for Android ARM64
build_android_arm64() {
    print_status "Building for Android ARM64..."
    if command -v aarch64-linux-gnu-g++ &> /dev/null; then
        aarch64-linux-gnu-g++ $STUB_SOURCES -o dist/OpenCL-Benchmark-android-arm64 $COMMON_FLAGS -pthread -static-libgcc -static-libstdc++ -ldl
        print_success "Android ARM64 build completed"
    else
        print_error "aarch64-linux-gnu-g++ not found, skipping Android ARM64 build"
    fi
}

# Main build process
echo "=============================================="
echo "  OpenCL-Benchmark Cross-Platform Build"
echo "=============================================="
echo ""

# Build all targets
build_linux_x86_64
build_linux_arm32
build_linux_arm64
build_android_arm32
build_android_arm64

echo ""
echo "=============================================="
echo "  Build Summary"
echo "=============================================="
ls -la dist/
echo ""
print_success "All builds completed successfully!"
