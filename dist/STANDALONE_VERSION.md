# OpenCL-Benchmark 故障排除 - 设备无 OpenCL 库

## 问题说明

执行测试程序时提示：
```
Error: No OpenCL devices are available.
```

**根本原因：** Android 设备上 `libGLES_mali.so` 是 Mali GPU 的 OpenGL ES 驱动，**不提供 OpenCL ICD 接口**。原版测试程序通过标准的 `libOpenCL.so` (OpenCL ICD) 加载 OpenCL 设备，但很多 Android 设备没有 `libOpenCL.so`。

## 解决方案：使用动态加载版本

我们创建了 `OpenCL-Benchmark-standalone` 版本，它使用 `dlopen` 动态加载 OpenCL 库，可以从多个常见库名加载（`libGLES_mali.so`、`libOpenCL.so` 等）。

### 推送新版本

```bash
# 1. 下载最新文件
git clone https://github.com/kukakong/OpenCL-Benchmark.git
cd OpenCL-Benchmark/dist

# 2. 推送独立版本到设备
adb push OpenCL-Benchmark-standalone-android-arm64 /data/local/tmp/
adb shell chmod +x /data/local/tmp/OpenCL-Benchmark-standalone-android-arm64

# 3. 运行
adb shell /data/local/tmp/OpenCL-Benchmark-standalone-android-arm64
```

### 独立版本特性

1. **不依赖 OpenCL 头文件** - 完全自包含
2. **运行时动态加载** - 通过 `dlopen` 加载 OpenCL
3. **支持多种库名**：
   - `libOpenCL.so` / `libOpenCL.so.1` (标准 ICD)
   - `libmali.so` / `libMali.so`
   - `libGLES_mali.so` (Mali 驱动)
   - `libPOCL.so` (CPU OpenCL)
4. **友好错误信息** - 明确提示缺少的库

## 如果仍然失败

### 1. 查找设备上所有可能的 OpenCL 库

```bash
adb shell
find / -name "libOpenCL*" 2>/dev/null
find / -name "libmali*" 2>/dev/null
find / -name "libMali*" 2>/dev/null
find /vendor -name "*.so" 2>/dev/null | xargs -I {} sh -c 'strings {} 2>/dev/null | grep -l "clGetPlatformIDs" 2>/dev/null && echo {}'
```

### 2. 尝试 CPU OpenCL 备选方案

如果设备没有任何 OpenCL 库，可以考虑：
- 使用 POCL (Portable OpenCL) - CPU 上运行 OpenCL
- 使用厂商专有驱动 (Adreno、PowerVR、Mali 都有专有 OpenCL)

### 3. 查看设备 GPU 类型

```bash
adb shell
cat /proc/cpuinfo | grep Hardware
getprop ro.hardware
getprop ro.board.platform
```

常见 GPU 厂商:
- **Mali (ARM)**: 需要 `libOpenCL.so` from ARM Compute Library
- **Adreno (Qualcomm)**: 需要 `libOpenCL.so` from Adreno SDK
- **PowerVR (Imagination)**: 需要 `libOpenCL.so` from PowerVR SDK

### 4. 安装 OpenCL 驱动

#### 方法 A: 厂商提供的 OpenCL

对于不同的 Android 设备，需要刷入对应厂商的 OpenCL 库：
- Samsung Exynos: `/system/vendor/lib/libOpenCL.so`
- Huawei Kirin: `/system/lib/libOpenCL.so`
- MediaTek: `/system/vendor/lib/libOpenCL.so`

#### 方法 B: 提取自其他 ROM

可以从相同芯片的官方 ROM 中提取 `libOpenCL.so`，然后推送到设备：

```bash
# 假设你从其他设备提取了 libOpenCL.so
adb push libOpenCL.so /data/local/tmp/
adb shell
# 创建符号链接
mkdir -p /data/local/tmp/lib
cp /data/local/tmp/libOpenCL.so /data/local/tmp/lib/
export LD_LIBRARY_PATH=/data/local/tmp/lib:$LD_LIBRARY_PATH
cd /data/local/tmp
./OpenCL-Benchmark-standalone-android-arm64
```

## 文件说明

| 文件 | 大小 | 说明 |
|------|------|------|
| `OpenCL-Benchmark-standalone-linux-x86_64` | 35K | Linux x86_64 独立版本 |
| `OpenCL-Benchmark-standalone-linux-arm32` | TBD | Linux ARM32 独立版本 |
| `OpenCL-Benchmark-standalone-linux-arm64` | TBD | Linux ARM64 独立版本 |
| `OpenCL-Benchmark-standalone-android-arm32` | TBD | Android ARM32 独立版本 |
| `OpenCL-Benchmark-standalone-android-arm64` | TBD | Android ARM64 独立版本 |

## 编译方法

如果需要在本地编译独立版本：

```bash
# 安装 Android NDK 后
NDK=/path/to/android-ndk-r21e
$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android21-clang++ \
    src/standalone_benchmark.cpp -o OpenCL-Benchmark-standalone-android-arm64 \
    -std=c++17 -O3 -static-libstdc++ -ldl
```

## 输出示例

成功时：
```
==============================================
  OpenCL-Benchmark (Standalone)
==============================================

Searching for OpenCL library...
  Loaded: libOpenCL.so
Found 1 platform(s)

Platform 0: ARM Platform
  Vendor: ARM
  Version: OpenCL 3.0
  Device: Mali-G78
    ...

==============================================
  Running Test on: Mali-G78
==============================================
Program compiled successfully!

--- Test 1: Vector Add ---
  Result: PASSED
  c[0] = 3, c[100] = 3

--- Test 2: Compute Benchmark ---
  Time: 0.523 ms
  Performance: 401.5 GFLOPS
```

失败时：
```
Searching for OpenCL library...
  Error: ...
Failed to load OpenCL library!

Please install OpenCL drivers:
  Android: libGLES_mali.so should be in /vendor/lib*/egl/
  Linux:   Install ocl-icd-libopencl1 or vendor-specific drivers
```
