#!/system/bin/sh
# OpenCL Device Diagnostic Script for Android
# Run this to diagnose why no OpenCL devices are found

echo "=============================================="
echo "  OpenCL Device Diagnostic"
echo "=============================================="
echo ""

# 1. Check OpenCL ICD loader
echo "1. Checking OpenCL ICD loader..."
if [ -f "/system/lib/libOpenCL.so" ] || [ -f "/system/lib64/libOpenCL.so" ] || \
   [ -f "/vendor/lib/libOpenCL.so" ] || [ -f "/vendor/lib64/libOpenCL.so" ]; then
    echo "  [OK] libOpenCL.so found"
    find /system /vendor -name "libOpenCL.so*" 2>/dev/null | while read f; do
        echo "    $f"
    done
else
    echo "  [WARN] libOpenCL.so not found in standard paths"
fi

# 2. Check ICD configuration directory
echo ""
echo "2. Checking ICD configuration..."
ICD_DIRS="/etc/OpenCL/vendors /vendor/etc/OpenCL/vendors /system/etc/OpenCL/vendors"
for dir in $ICD_DIRS; do
    if [ -d "$dir" ]; then
        echo "  [OK] Found ICD directory: $dir"
        ls -la "$dir" 2>/dev/null
        cat "$dir"/*.icd 2>/dev/null | while read line; do
            echo "    ICD config: $line"
        done
    fi
done

# 3. Search for Mali libraries
echo ""
echo "3. Searching for Mali GPU libraries..."
find /vendor /system -name "*mali*" -type f 2>/dev/null | while read f; do
    echo "  $f"
done

# 4. Check for OpenCL symbols in Mali libraries
echo ""
echo "4. Checking OpenCL symbols in Mali libraries..."
for lib in /vendor/lib*/libmali.so /vendor/lib*/libMali.so /vendor/lib*/egl/libGLES_mali.so; do
    if [ -f "$lib" ]; then
        echo "  Checking $lib..."
        if strings "$lib" 2>/dev/null | grep -q "clGetPlatformIDs"; then
            echo "    [OK] Contains OpenCL symbols!"
        else
            echo "    [INFO] No OpenCL symbols found"
        fi
    fi
done

# 5. Check GPU hardware
echo ""
echo "5. Checking GPU hardware..."
if [ -f "/proc/cpuinfo" ]; then
    echo "  Hardware: $(grep -m1 "Hardware" /proc/cpuinfo 2>/dev/null)"
fi
echo "  ro.hardware: $(getprop ro.hardware 2>/dev/null)"
echo "  ro.board.platform: $(getprop ro.board.platform 2>/dev/null)"

# 6. Check for GPU devices
echo ""
echo "6. Checking GPU devices..."
for gpu in /dev/mali /dev/mali0 /dev/kgsl-3d0 /dev/pvr_sync /dev/ion; do
    if [ -e "$gpu" ]; then
        ls -la "$gpu" 2>/dev/null
    fi
done

# 7. List all .so files in vendor lib that might be OpenCL related
echo ""
echo "7. Searching for potential OpenCL libraries..."
find /vendor/lib* -name "*.so" 2>/dev/null | xargs -I{} sh -c '
    if strings "{}" 2>/dev/null | grep -q "clGetPlatformIDs"; then
        echo "  [OPENCL] {}"
    fi
' 2>/dev/null

echo ""
echo "=============================================="
echo "  Diagnostic Complete"
echo "=============================================="
