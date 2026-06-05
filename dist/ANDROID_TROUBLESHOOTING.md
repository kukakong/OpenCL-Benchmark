# OpenCL-Benchmark Android 使用指南

## 问题诊断

### 问题：找不到 OpenCL 设备

```
Error: No OpenCL devices are available.
```

**原因：** 程序无法找到 OpenCL 库文件（通常是 Mali GPU 库）。

## 解决方案

### 方案一：使用智能启动脚本（推荐）

```bash
# 推送文件
adb push OpenCL-Benchmark-android-arm64 /data/local/tmp/
adb push run-android-auto.sh /data/local/tmp/

# 赋予权限
adb shell chmod +x /data/local/tmp/*

# 运行（脚本会自动查找 OpenCL 库）
adb shell /data/local/tmp/run-android-auto.sh
```

### 方案二：手动设置库路径

```bash
# 进入 Android shell
adb shell

# 设置库路径（根据你的设备选择）
# ARM64 设备：
export LD_LIBRARY_PATH=/vendor/lib64:/vendor/lib64/egl:/system/lib64:$LD_LIBRARY_PATH

# ARM32 设备：
export LD_LIBRARY_PATH=/vendor/lib:/vendor/lib/egl:/system/lib:$LD_LIBRARY_PATH

# 运行程序
cd /data/local/tmp
./OpenCL-Benchmark-android-arm64
```

### 方案三：预加载 Mali 库

```bash
# 查找 Mali 库位置
find /vendor -name "libGLES_mali.so" 2>/dev/null

# 预加载并运行
export LD_PRELOAD=/vendor/lib64/egl/libGLES_mali.so
export LD_LIBRARY_PATH=/vendor/lib64:/vendor/lib64/egl:$LD_LIBRARY_PATH
./OpenCL-Benchmark-android-arm64
```

## 常见库位置

### ARM64 设备
```
/vendor/lib64/egl/libGLES_mali.so    # 最常见
/vendor/lib64/libmali.so
/vendor/lib64/libMali.so
/vendor/lib64/hw/gralloc.mali.so
```

### ARM32 设备
```
/vendor/lib/egl/libGLES_mali.so      # 最常见
/vendor/lib/libmali.so
/vendor/lib/libMali.so
/vendor/lib/hw/gralloc.mali.so
```

## 验证步骤

### 1. 检查设备是否有 Mali GPU

```bash
adb shell cat /proc/cpuinfo | grep -i hardware
adb shell getprop ro.hardware
```

### 2. 查找 GPU 库文件

```bash
adb shell find /vendor -name "*mali*" 2>/dev/null
adb shell find /system -name "*OpenCL*" 2>/dev/null
adb shell ls -la /vendor/lib*/egl/
```

### 3. 检查库依赖

```bash
# 在设备上运行
adb shell
cd /data/local/tmp

# 查看程序需要的库
readelf -d OpenCL-Benchmark-android-arm64 | grep NEEDED

# 查看库是否可找到
readelf -d /vendor/lib64/egl/libGLES_mali.so | grep SONAME
```

## 创建符号链接（如果需要）

如果系统有 Mali 库但没有 libOpenCL.so，可以创建符号链接：

```bash
# 需要root权限
adb root
adb remount

# 创建符号链接
adb shell ln -s /vendor/lib64/egl/libGLES_mali.so /vendor/lib64/libOpenCL.so
adb shell ln -s /vendor/lib64/egl/libGLES_mali.so /vendor/lib64/libOpenCL.so.1
```

## 完整示例

### 示例 1：标准 Android 设备

```bash
# 推送文件
adb push OpenCL-Benchmark-android-arm64 /data/local/tmp/
adb push run-android-auto.sh /data/local/tmp/

# 运行
adb shell
cd /data/local/tmp
chmod +x *
./run-android-auto.sh
```

### 示例 2：需要 root 的设备

```bash
# 获取 root
adb root
adb remount

# 创建库链接
adb shell ln -sf /vendor/lib64/egl/libGLES_mali.so /system/lib64/libOpenCL.so.1

# 运行
adb push OpenCL-Benchmark-android-arm64 /data/local/tmp/
adb shell /data/local/tmp/OpenCL-Benchmark-android-arm64
```

### 示例 3：使用 su 运行

```bash
adb push OpenCL-Benchmark-android-arm64 /data/local/tmp/
adb shell su -c "chmod +x /data/local/tmp/OpenCL-Benchmark-android-arm64"
adb shell su -c "export LD_LIBRARY_PATH=/vendor/lib64:/vendor/lib64/egl:\$LD_LIBRARY_PATH && /data/local/tmp/OpenCL-Benchmark-android-arm64"
```

## 故障排除

### 问题：Permission denied

```bash
chmod +x /data/local/tmp/OpenCL-Benchmark-android-arm64
```

### 问题：library not found

```bash
# 查看缺少的库
LD_DEBUG=libs ./OpenCL-Benchmark-android-arm64 2>&1 | head -50

# 或使用 strace
strace -e openat ./OpenCL-Benchmark-android-arm64 2>&1 | grep -i opencl
```

### 问题：No such file or directory

这通常是动态链接器问题，确保使用正确的架构：
- ARM64 设备使用 `OpenCL-Benchmark-android-arm64`
- ARM32 设备使用 `OpenCL-Benchmark-android-arm32`

```bash
# 检查设备架构
adb shell uname -m
```

## 性能优化

### 设置 CPU 频率（需要 root）

```bash
# 设置为性能模式
adb shell su -c "echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"
```

### 设置 GPU 频率（需要 root）

```bash
# 查找 GPU 频率控制
adb shell find /sys -name "*gpu*" -type d 2>/dev/null

# 设置最大频率（具体路径因设备而异）
adb shell su -c "echo 900000000 > /sys/class/kgsl/kgsl-0/max_gpuclk"
```

## 文件列表

| 文件 | 说明 |
|------|------|
| `OpenCL-Benchmark-android-arm64` | ARM64 可执行文件 |
| `OpenCL-Benchmark-android-arm32` | ARM32 可执行文件 |
| `run-android-auto.sh` | 智能启动脚本（自动查找库） |
| `run-android.sh` | 基础启动脚本 |
| `run-android-arm64.sh` | ARM64 专用启动脚本 |
| `run-android-arm32.sh` | ARM32 专用启动脚本 |

## 版本信息

- 编译工具：Android NDK r21e
- 目标 API：Android 5.0 (API 21) 及以上
- 支持架构：ARM32, ARM64
- GPU 支持：Mali GPU, 其他 OpenCL 兼容 GPU
