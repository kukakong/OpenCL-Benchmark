# OpenCL 设备未找到 - 完整诊断指南

## 问题现象

```
Platform 0: ARM Platform
  Vendor: ARM
  Version: 

No OpenCL devices found!
```

程序成功加载了 `libOpenCL.so`，找到了 ARM Platform，但没有找到任何设备。

## 根本原因

**OpenCL ICD 加载器找到了，但没有实际的 OpenCL 驱动被注册。**

OpenCL 使用两层架构：
1. **ICD 加载器** (`libOpenCL.so`) - 负责加载所有厂商驱动
2. **厂商驱动** (`libmali.so`, `libnvidia-opencl.so` 等) - 实际的 GPU 驱动

你的系统有 ICD 加载器，但厂商驱动没有被正确注册到 ICD。

## 诊断步骤

### 步骤 1：运行诊断脚本

```bash
# 推送诊断脚本
adb push diagnose_opencl.sh /data/local/tmp/
adb shell chmod +x /data/local/tmp/diagnose_opencl.sh

# 运行诊断
adb shell /data/local/tmp/diagnose_opencl.sh
```

### 步骤 2：检查 ICD 配置文件

ICD 加载器需要配置文件来找到厂商驱动：

```bash
adb shell
ls -la /vendor/etc/OpenCL/vendors/
ls -la /etc/OpenCL/vendors/
```

**正确的配置应该包含 `.icd` 文件：**
```
/vendor/etc/OpenCL/vendors/mali.icd
```

**文件内容示例：**
```
libmali.so
```

### 步骤 3：检查 Mali 库是否包含 OpenCL

```bash
adb shell
# 检查 Mali 库是否导出 OpenCL 函数
strings /vendor/lib64/libmali.so | grep clGetPlatformIDs
strings /vendor/lib64/egl/libGLES_mali.so | grep clGetPlatformIDs
```

如果输出包含 `clGetPlatformIDs`，说明这个库支持 OpenCL。

## 解决方案

### 方案 A：创建 ICD 配置文件（需要 root）

如果 Mali 库包含 OpenCL 符号，但缺少 ICD 配置：

```bash
adb root
adb remount

# 创建 ICD 配置目录
adb shell mkdir -p /vendor/etc/OpenCL/vendors

# 创建配置文件，指向 Mali 库
adb shell "echo 'libmali.so' > /vendor/etc/OpenCL/vendors/mali.icd"

# 或者如果库在 egl 子目录
adb shell "echo '/vendor/lib64/egl/libGLES_mali.so' > /vendor/etc/OpenCL/vendors/mali.icd"

# 重启以生效
adb reboot
```

### 方案 B：创建符号链接（需要 root）

如果系统期望 `libOpenCL.so` 直接是 Mali 库：

```bash
adb root
adb remount

# 备份原 ICD 加载器
adb shell mv /vendor/lib64/libOpenCL.so /vendor/lib64/libOpenCL-icd.so

# 创建指向 Mali 的符号链接
adb shell ln -s /vendor/lib64/libmali.so /vendor/lib64/libOpenCL.so

# 重启
adb reboot
```

### 方案 C：使用 LD_PRELOAD 强制加载

不需要 root，但需要每次运行时设置：

```bash
adb shell
export LD_PRELOAD=/vendor/lib64/libmali.so
export LD_LIBRARY_PATH=/vendor/lib64:/vendor/lib64/egl:$LD_LIBRARY_PATH
cd /data/local/tmp
./OpenCL-Benchmark-standalone-android-arm32
```

### 方案 D：修改程序直接加载 Mali 库

如果上述方案都不行，可以修改 `standalone_benchmark.cpp`，让它直接尝试加载 Mali 库：

```cpp
// 在 load() 函数中，把 Mali 库放在最前面
const char* lib_names[] = {
    "/vendor/lib64/libmali.so",           // 直接路径
    "/vendor/lib64/egl/libGLES_mali.so",  // Mali egl 路径
    "/vendor/lib/libmali.so",             // 32位路径
    "libmali.so",
    "libMali.so",
    "libOpenCL.so",
    // ... 其他
};
```

## 常见厂商配置

### ARM Mali GPU

ICD 配置文件位置：
```
/vendor/etc/OpenCL/vendors/mali.icd
```

内容：
```
libmali.so
```

或完整路径：
```
/vendor/lib64/libmali.so
```

### Qualcomm Adreno GPU

ICD 配置文件：
```
/vendor/etc/OpenCL/vendors/adreno.icd
```

内容：
```
libadreno_cl.so
```

### 检查你的设备

```bash
# 查找所有可能的 OpenCL 库
adb shell find /vendor -name "*.so" 2>/dev/null | xargs -I{} sh -c 'strings {} 2>/dev/null | grep -q clGetPlatformIDs && echo {}'

# 查找 ICD 配置
adb shell find / -name "*.icd" 2>/dev/null

# 检查环境变量
adb shell cat /vendor/etc/mali_platform.config 2>/dev/null
```

## 验证修复

修复后运行：

```bash
adb shell /data/local/tmp/OpenCL-Benchmark-standalone-android-arm32
```

应该看到：
```
Platform 0: ARM Platform
  Vendor: ARM
  Version: OpenCL 3.0
  Device: Mali-Gxx
    ...
```

## 如果仍然失败

请收集以下信息并反馈：

```bash
# 1. 设备信息
adb shell getprop ro.hardware
adb shell getprop ro.board.platform
adb shell getprop ro.product.model

# 2. GPU 信息
adb shell cat /proc/cpuinfo | grep Hardware
adb shell ls -la /dev/mali* /dev/kgsl* 2>/dev/null

# 3. Mali 库信息
adb shell ls -la /vendor/lib*/libmali* /vendor/lib*/egl/libGLES_mali* 2>/dev/null

# 4. ICD 配置
adb shell ls -laR /vendor/etc/OpenCL /etc/OpenCL 2>/dev/null

# 5. 检查库符号
adb shell strings /vendor/lib64/libmali.so 2>/dev/null | grep -E "clGet|clCreate"
```
