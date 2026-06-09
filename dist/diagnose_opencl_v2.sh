#!/vendor/bin/sh
# OpenCL Device Diagnostic Script for Android (Architecture-Aware)
# This version handles 32/64-bit library mismatches

echo "=============================================="
echo "  OpenCL Device Diagnostic (Arch-Aware)"
echo "=============================================="
echo ""

# Detect current architecture
ARCH=$(uname -m)
echo "Current architecture: $ARCH"

# Set library paths based on architecture
case "$ARCH" in
    aarch64|arm64)
        LIB_DIRS="/vendor/lib64 /system/lib64 /vendor/lib64/egl"
        ALT_LIB_DIRS="/vendor/lib /system/lib /vendor/lib/egl"
        echo "  (Detected 64-bit system)"
        ;;
    armv7*|armhf)
        LIB_DIRS="/vendor/lib /system/lib /vendor/lib/egl"
        ALT_LIB_DIRS="/vendor/lib64 /system/lib64 /vendor/lib64/egl"
        echo "  (Detected 32-bit system)"
        ;;
esac

# 1. Check for libOpenCL.so
echo ""
echo "1. Checking for libOpenCL.so..."
FOUND_OPENCL=""
for dir in $LIB_DIRS; do
    if [ -f "$dir/libOpenCL.so" ]; then
        echo "  [OK] Found: $dir/libOpenCL.so"
        FOUND_OPENCL="$dir/libOpenCL.so"
        break
    fi
done
if [ -z "$FOUND_OPENCL" ]; then
    echo "  [WARN] libOpenCL.so not found in native arch paths"
    echo "  Checking alternate (cross-arch) paths..."
    for dir in $ALT_LIB_DIRS; do
        if [ -f "$dir/libOpenCL.so" ]; then
            echo "  [INFO] Found in alternate path: $dir/libOpenCL.so"
            echo "         (This is a cross-arch mismatch!)"
        fi
    done
fi

# 2. Check for Mali libraries
echo ""
echo "2. Checking for Mali GPU libraries..."
FOUND_MALI=""
for dir in $LIB_DIRS; do
    for lib in libmali.so libMali.so; do
        if [ -f "$dir/$lib" ]; then
            echo "  [OK] Found: $dir/$lib"
            FOUND_MALI="$dir/$lib"
            break 2
        fi
    done
    if [ -d "$dir/egl" ]; then
        if [ -f "$dir/egl/libGLES_mali.so" ]; then
            echo "  [OK] Found: $dir/egl/libGLES_mali.so"
            FOUND_MALI="$dir/egl/libGLES_mali.so"
            break
        fi
    fi
done

if [ -z "$FOUND_MALI" ]; then
    echo "  [WARN] No Mali library found in native arch paths"
    echo "  Checking alternate paths..."
    for dir in $ALT_LIB_DIRS; do
        if [ -f "$dir/libmali.so" ] || [ -f "$dir/libMali.so" ]; then
            echo "  [INFO] Found Mali in alternate path: $dir"
            echo "         (Architecture mismatch - cannot use directly)"
        fi
        if [ -d "$dir/egl" ] && [ -f "$dir/egl/libGLES_mali.so" ]; then
            echo "  [INFO] Found Mali EGL in alternate path: $dir/egl/libGLES_mali.so"
            echo "         (Architecture mismatch - 32-bit lib on 64-bit system)"
        fi
    done
fi

# 3. Check ICD configuration
echo ""
echo "3. Checking ICD configuration..."
ICD_FOUND=0
for dir in /vendor/etc/OpenCL/vendors /etc/OpenCL/vendors /system/etc/OpenCL/vendors; do
    if [ -d "$dir" ]; then
        echo "  [OK] ICD directory: $dir"
        ls -la "$dir" 2>/dev/null
        for icd in "$dir"/*.icd; do
            if [ -f "$icd" ]; then
                echo "  [OK] ICD file: $icd"
                cat "$icd" 2>/dev/null
                ICD_FOUND=1
            fi
        done
    fi
done
if [ $ICD_FOUND -eq 0 ]; then
    echo "  [WARN] No ICD configuration files found!"
    echo "         This is likely why no devices are detected."
fi

# 4. Check GPU hardware
echo ""
echo "4. Checking GPU hardware..."
if [ -f "/proc/cpuinfo" ]; then
    grep -m1 "Hardware" /proc/cpuinfo 2>/dev/null
fi
getprop ro.hardware 2>/dev/null | while read val; do echo "  ro.hardware: $val"; done
getprop ro.board.platform 2>/dev/null | while read val; do echo "  ro.board.platform: $val"; done

# 5. Check GPU devices
echo ""
echo "5. Checking GPU device nodes..."
for dev in /dev/mali /dev/mali0 /dev/kgsl-3d0 /dev/pvr_sync /dev/ump /dev/ion; do
    if [ -e "$dev" ]; then
        ls -la "$dev" 2>/dev/null
    fi
done

# 6. Architecture mismatch warning
echo ""
echo "6. Architecture analysis..."
if [ "$ARCH" = "aarch64" ] || [ "$ARCH" = "arm64" ]; then
    if [ -f "/vendor/lib/egl/libGLES_mali.so" ] && [ ! -f "/vendor/lib64/egl/libGLES_mali.so" ]; then
        echo "  [CRITICAL] Architecture mismatch detected!"
        echo "  - System is 64-bit ($ARCH)"
        echo "  - Mali library is 32-bit (/vendor/lib/egl/libGLES_mali.so)"
        echo "  - No 64-bit Mali library found"
        echo ""
        echo "  SOLUTION: Use 32-bit benchmark binary:"
        echo "    ./OpenCL-Benchmark-standalone-android-arm32"
    fi
fi

echo ""
echo "=============================================="
echo "  Diagnostic Complete"
echo "=============================================="
echo ""
echo "Recommendations:"
if [ -z "$FOUND_MALI" ] && [ -z "$FOUND_OPENCL" ]; then
    echo "  - No OpenCL/Mali libraries found for your architecture"
    echo "  - Check if GPU drivers are properly installed"
elif [ $ICD_FOUND -eq 0 ]; then
    echo "  - ICD configuration missing"
    echo "  - Create /vendor/etc/OpenCL/vendors/mali.icd with content: libmali.so"
fi
