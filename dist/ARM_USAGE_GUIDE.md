# OpenCL-Benchmark ARM 平台使用指南

## 概述

本指南详细说明如何在 Linux ARM 和 Android ARM 平台上运行 OpenCL-Benchmark 测试程序。程序已针对 Mali GPU 进行优化，支持动态库加载。

---

## 文件列表

### 可执行文件
| 文件名 | 目标平台 | 说明 |
|--------|----------|------|
| `OpenCL-Benchmark-linux-arm32` | Linux ARM32 | 32位 ARM Linux 系统 |
| `OpenCL-Benchmark-linux-arm64` | Linux ARM64 | 64位 ARM Linux 系统 |
| `OpenCL-Benchmark-android-arm32` | Android ARM32 | 32位 Android 设备 |
| `OpenCL-Benchmark-android-arm64` | Android ARM64 | 64位 Android 设备 |

### 启动脚本
| 文件名 | 说明 |
|--------|------|
| `run-linux.sh` | Linux 智能启动脚本（自动检测架构和库路径） |
| `run-android.sh` | Android 智能启动脚本（自动检测架构和库路径） |
| `run-linux-arm32.sh` | Linux ARM32 专用启动脚本 |
| `run-linux-arm64.sh` | Linux ARM64 专用启动脚本 |
| `run-android-arm32.sh` | Android ARM32 专用启动脚本 |
| `run-android-arm64.sh` | Android ARM64 专用启动脚本 |

---

## 库依赖说明

### Linux 平台 Mali GPU 库位置

Mali GPU 驱动库通常位于以下位置：

```
# 常见库位置
/lib/libmali.so
/lib64/libmali.so
/lib/libMali.so
/lib64/libMali.so
/usr/lib/libmali.so
/usr/lib/libMali.so
/usr/lib64/libmali.so
/usr/lib64/libMali.so
```

这些库文件通常提供 OpenCL、OpenGL ES 和 Vulkan 支持。

### Android 平台 Mali GPU 库位置

Android 设备上的 Mali GPU 库通常位于：

```
# ARM32 设备
/vendor/lib/egl/libGLES_mali.so
/vendor/lib/hw/gralloc.mali.so

# ARM64 设备
/vendor/lib64/egl/libGLES_mali.so
/vendor/lib64/hw/gralloc.mali.so
```

---

## 使用方法

### Linux ARM 平台

#### 方法一：使用智能启动脚本（推荐）

```bash
# 赋予执行权限
chmod +x run-linux.sh
chmod +x OpenCL-Benchmark-linux-arm*

# 运行测试（自动检测架构）
./run-linux.sh

# 测试指定设备
./run-linux.sh 0 1
```

#### 方法二：使用专用启动脚本

```bash
# ARM64 系统
chmod +x run-linux-arm64.sh OpenCL-Benchmark-linux-arm64
./run-linux-arm64.sh

# ARM32 系统
chmod +x run-linux-arm32.sh OpenCL-Benchmark-linux-arm32
./run-linux-arm32.sh
```

#### 方法三：手动设置库路径

```bash
# 设置库路径
export LD_LIBRARY_PATH=/lib:/lib64:/usr/lib:/usr/lib64:$LD_LIBRARY_PATH

# 直接运行
./OpenCL-Benchmark-linux-arm64
```

### Android 平台

#### 方法一：使用 ADB 推送并运行

```bash
# 推送所有文件到设备
adb push OpenCL-Benchmark-android-arm64 /data/local/tmp/
adb push run-android.sh /data/local/tmp/

# 赋予执行权限
adb shell chmod +x /data/local/tmp/OpenCL-Benchmark-android-arm64
adb shell chmod +x /data/local/tmp/run-android.sh

# 运行测试
adb shell /data/local/tmp/run-android.sh
```

#### 方法二：使用专用启动脚本

```bash
# 推送文件
adb push OpenCL-Benchmark-android-arm64 /data/local/tmp/
adb push run-android-arm64.sh /data/local/tmp/

# 运行
adb shell chmod +x /data/local/tmp/*
adb shell /data/local/tmp/run-android-arm64.sh
```

#### 方法三：手动设置库路径

```bash
# 进入设备 shell
adb shell

# 设置库路径
export LD_LIBRARY_PATH=/vendor/lib64:/vendor/lib64/egl:/system/lib64:$LD_LIBRARY_PATH

# 运行
cd /data/local/tmp
./OpenCL-Benchmark-android-arm64
```

---

## 故障排除

### 问题 1：找不到库文件

**错误信息：**
```
error while loading shared libraries: libOpenCL.so.1: cannot open shared object file
```

**解决方法：**

1. **查找库文件位置：**
   ```bash
   # Linux
   find / -name "libmali.so" 2>/dev/null
   find / -name "libOpenCL.so*" 2>/dev/null
   
   # Android (需要 root)
   find /vendor -name "libGLES_mali.so"
   ```

2. **设置正确的库路径：**
   ```bash
   export LD_LIBRARY_PATH=/找到的库路径:$LD_LIBRARY_PATH
   ```

3. **创建符号链接（如果需要）：**
   ```bash
   # 如果系统有 libmali.so 但没有 libOpenCL.so
   ln -s /lib/libmali.so /lib/libOpenCL.so
   ln -s /lib/libmali.so /lib/libOpenCL.so.1
   ```

### 问题 2：权限不足

**错误信息：**
```
Permission denied
```

**解决方法：**
```bash
chmod +x OpenCL-Benchmark-*
chmod +x run-*.sh
```

### 问题 3：找不到 OpenCL 设备

**错误信息：**
```
No OpenCL devices are available
```

**解决方法：**

1. 确认 GPU 驱动已正确安装
2. 检查库文件是否存在
3. 验证库路径设置正确

**Linux 安装 Mali 驱动：**
```bash
# Debian/Ubuntu
sudo apt-get install mali-fbdev
# 或
sudo apt-get install libmali-xr11
```

### 问题 4：Android 需要 Root 权限

某些 Android 设备需要 root 权限才能访问 GPU：

```bash
# 检查是否有 root
adb shell su -c "id"

# 使用 root 运行
adb shell su -c "/data/local/tmp/run-android.sh"
```

---

## 性能测试项目

### 计算性能测试
- **FP64** - 64位双精度浮点运算
- **FP32** - 32位单精度浮点运算
- **FP16** - 16位半精度浮点运算
- **INT64** - 64位整数运算
- **INT32** - 32位整数运算
- **INT16** - 16位整数运算
- **INT8** - 8位整数运算 (dp4a)

### 内存带宽测试
- 合并读取/写入带宽
- 非对齐读取/写入带宽

### PCIe 带宽测试（仅独立 GPU）
- 发送/接收/双向带宽

---

## 示例输出

```
==============================================
  OpenCL-Benchmark Launcher
==============================================
Architecture: aarch64 (arm64)
Library Path: /lib:/lib64:/usr/lib:/usr/lib64
==============================================
.-----------------------------------------------------------------------------.
|----------------.------------------------------------------------------------|
| Device ID      | 0                                                          |
| Device Name    | Mali-G52 MP2                                               |
| Device Vendor  | ARM                                                        |
| Device Driver  | 1.0 (Linux)                                                |
| OpenCL Version | OpenCL C 2.0                                               |
| Compute Units  | 2 at 600 MHz (16 cores, 0.019 TFLOPs/s)                    |
| Memory, Cache  | 1024 MB RAM, 64 KB global / 32 KB local                    |
| Buffer Limits  | 1024 MB global, 64 KB constant                             |
|----------------'------------------------------------------------------------|
| Info: OpenCL C code successfully compiled.                                  |
| FP64   Compute   (double, fma  )                       not supported        |
| FP32   Compute   (float , fma  )                      0.018 TFLOPs/s ( 1x )|
| FP16   Compute   (half2 , fma  )                      0.035 TFLOPs/s ( 2x )|
| INT64  Compute   (long  , a*b+c)                      0.009  TIOPs/s (1/2 )|
| INT32  Compute   (int   , a*b+c)                      0.018  TIOPs/s ( 1x )|
| INT16  Compute   (short2, a*b+c)                      0.035  TIOPs/s ( 2x )|
| INT8   Compute   (char4 , dp4a )                      0.071  TIOPs/s ( 4x )|
| Memory Bandwidth ( coalesced read      )                       5.12 GB/s |
| Memory Bandwidth ( coalesced      write)                       4.98 GB/s |
| Memory Bandwidth (misaligned read      )                       2.45 GB/s |
| Memory Bandwidth (misaligned      write)                       1.87 GB/s |
|-----------------------------------------------------------------------------|
'-----------------------------------------------------------------------------'
```

---

## 高级配置

### 创建本地库目录

如果系统库位置不标准，可以创建本地 lib 目录：

```bash
# 创建 lib 目录
mkdir -p lib

# 复制或链接库文件
ln -s /path/to/libmali.so lib/libOpenCL.so.1
```

### 环境变量配置

```bash
# 设置 OpenCL 平台
export OCL_PLATFORM=ARM

# 设置设备类型
export OCL_DEVICE_TYPE=GPU

# 设置库路径
export LD_LIBRARY_PATH=/custom/lib/path:$LD_LIBRARY_PATH
```

---

## 版本信息

- 编译日期：2026-06-04
- 目标平台：Linux ARM32/ARM64, Android ARM32/ARM64
- OpenCL 版本：支持 1.0 - 3.0
- GPU 支持：Mali GPU (ARM), 其他 OpenCL 兼容 GPU

---

## 许可证

本项目遵循原 OpenCL-Benchmark 项目的许可证。
