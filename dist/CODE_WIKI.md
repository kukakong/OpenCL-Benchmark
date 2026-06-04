# OpenCL-Benchmark Code Wiki

## 项目概述

OpenCL-Benchmark 是一个跨平台的 OpenCL 性能基准测试工具，用于测量 GPU/CPU 的峰值计算性能、内存带宽和 PCIe 带宽。该项目支持 Windows、Linux、macOS 和 Android 平台，能够与任何支持 OpenCL 的 GPU 配合工作。

---

## 项目整体架构

```
OpenCL-Benchmark/
├── src/                          # 源代码目录
│   ├── main.cpp                  # 主程序入口，基准测试逻辑
│   ├── kernel.cpp                # OpenCL C 内核代码定义
│   ├── kernel.hpp                # OpenCL 内核头文件和宏定义
│   ├── opencl.hpp                # OpenCL 封装层（核心模块）
│   ├── utilities.hpp             # 通用工具函数库
│   ├── opencl_stub.cpp           # OpenCL stub库（用于交叉编译）
│   └── OpenCL/                   # OpenCL 头文件和库
│       ├── include/CL/           # OpenCL API 头文件
│       └── lib/                  # OpenCL 库文件
├── dist/                         # 编译输出目录
│   ├── OpenCL-Benchmark-linux-x86_64
│   ├── OpenCL-Benchmark-linux-arm32
│   ├── OpenCL-Benchmark-linux-arm64
│   ├── OpenCL-Benchmark-android-arm32
│   └── OpenCL-Benchmark-android-arm64
├── build_all.sh                  # 跨平台編譯腳本
├── make.sh                       # 原始編譯腳本
└── README.md                     # 項目說明文檔
```

### 架构层次图

```
┌─────────────────────────────────────────────────────────────┐
│                     Application Layer                        │
│                        (main.cpp)                            │
│  - 基准测试流程控制                                           │
│  - 设备选择和测试执行                                         │
│  - 结果格式化输出                                             │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                     OpenCL Wrapper Layer                     │
│                      (opencl.hpp)                            │
│  - Device: 设备管理                                           │
│  - Memory: 内存缓冲区管理                                     │
│  - Kernel: 内核执行管理                                       │
│  - Device_Info: 设备信息查询                                  │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                     Kernel Definition Layer                   │
│                   (kernel.hpp/cpp)                           │
│  - OpenCL C 内核代码                                          │
│  - 计算内核 (FP64/FP32/FP16/INT64/INT32/INT16/INT8)          │
│  - 内存带宽测试内核                                           │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                     Utilities Layer                           │
│                    (utilities.hpp)                           │
│  - 字符串处理                                                 │
│  - 数学运算                                                   │
│  - 文件操作                                                   │
│  - 控制台输出                                                 │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                     OpenCL API Layer                          │
│                 (OpenCL Headers & Library)                   │
│  - Khronos OpenCL C++ API                                     │
│  - 平台抽象层                                                 │
└─────────────────────────────────────────────────────────────┘
```

---

## 主要模块职责

### 1. main.cpp - 主程序模块

**职责**：基准测试的主入口和测试流程控制

**主要功能**：
- 设备枚举和选择
- 基准测试执行
- 性能结果计算和显示
- 多设备测试支持

**关键函数**：

| 函数名 | 功能描述 |
|--------|----------|
| `fraction()` | 将性能比值转换为最接近的分数表示 |
| `benchmark_device()` | 对指定设备执行完整的基准测试 |
| `main()` | 程序入口，解析参数并启动测试 |

### 2. opencl.hpp - OpenCL 封装模块

**职责**：提供高级 OpenCL API 封装，简化设备、内存和内核管理

**主要组件**：

#### Device_Info 结构体
设备信息查询和存储，包含：
- 设备基本信息（名称、厂商、驱动版本）
- 计算能力信息（计算单元、时钟频率、核心数）
- 内存信息（全局内存、缓存大小）
- 性能能力（FP64/FP32/FP16 支持）

#### Device 类
设备管理类，负责：
- OpenCL 程序编译
- 命令队列管理
- 设备能力检测和补丁应用

#### Memory 模板类
内存缓冲区管理，支持：
- 主机/设备内存分配
- 零拷贝优化（CPU/iGPU）
- 多维数组访问
- 异步数据传输

#### Kernel 类
内核执行管理，提供：
- 内核参数绑定
- 工作组大小设置
- 内核执行控制

### 3. kernel.hpp/cpp - 内核定义模块

**职责**：定义 OpenCL C 内核代码

**内核类型**：

| 内核名称 | 测试类型 | 说明 |
|----------|----------|------|
| `kernel_double` | FP64 计算 | double 精度 FMA 运算 |
| `kernel_float` | FP32 计算 | float 精度 FMA 运算 |
| `kernel_half` | FP16 计算 | half2 精度运算 |
| `kernel_long` | INT64 计算 | long 整数运算 |
| `kernel_int` | INT32 计算 | int 整数运算 |
| `kernel_short` | INT16 计算 | short2 整数运算 |
| `kernel_char` | INT8 计算 | char4 dp4a 运算 |
| `kernel_coalesced_read/write` | 内存带宽 | 合并访问模式 |
| `kernel_misaligned_read/write` | 内存带宽 | 非对齐访问模式 |

### 4. utilities.hpp - 工具库模块

**职责**：提供通用工具函数和类型定义

**主要功能类别**：

#### 类型定义
```cpp
typedef unsigned char  uchar;
typedef unsigned short ushort;
typedef unsigned int   uint;
typedef int64_t        slong;
typedef uint64_t       ulong;
```

#### 数学函数
- `sq()`, `cb()`, `pow()` - 幂运算
- `clamp()`, `sign()` - 数值处理
- `rsqrt()`, `ln()` - 数学函数

#### 字符串处理
- `to_string()` - 数值转字符串（高精度）
- `split_regex()` - 正则分割
- `replace()` - 字符串替换
- `trim()`, `to_lower()`, `to_upper()` - 字符串处理

#### 输出函数
- `print()`, `println()` - 控制台输出
- `print_info()`, `print_warning()`, `print_error()` - 格式化消息

---

## 关键类与函数说明

### Device_Info 结构体

```cpp
struct Device_Info {
    cl::Device cl_device;           // OpenCL 设备对象
    cl::Context cl_context;         // OpenCL 上下文
    uint id;                        // 设备 ID
    string name, vendor;            // 设备名称、厂商
    string driver_version;          // 驱动版本
    string opencl_c_version;        // OpenCL C 版本
    uint memory;                    // 全局内存 (MB)
    uint compute_units;             // 计算单元数
    uint cores;                     // 核心数
    float tflops;                   // 估算 FP32 性能 (TFLOPs/s)
    bool is_cpu, is_gpu;            // 设备类型标志
    // ... 更多属性
};
```

**关键方法**：
- 构造函数：从 OpenCL 设备对象提取所有信息
- 自动检测厂商特定属性（AMD/Intel/NVIDIA）

### Device 类

```cpp
class Device {
public:
    Device_Info info;               // 设备信息
    
    // 构造函数
    Device(const Device_Info& info, const string& opencl_c_code);
    
    // 方法
    void barrier();                 // 屏障同步
    void finish_queue();            // 完成队列
    cl::Context get_cl_context();   // 获取上下文
    cl::Program get_cl_program();   // 获取程序
    cl::CommandQueue get_cl_queue(); // 获取命令队列
};
```

**设备能力补丁**：
- NVIDIA FP16 支持补丁
- Intel >4GB 内存支持
- ARM GPU FMA 性能优化

### Memory 模板类

```cpp
template<typename T>
class Memory {
public:
    // 多维访问指针
    T *x, *y, *z, *w;
    T *s0, *s1, ..., *sF;
    
    // 构造函数
    Memory(Device& device, const ulong N, const uint dimensions=1u,
           const bool allocate_host=true, const bool allocate_device=true,
           const T value=0, const bool allow_zero_copy=true);
    
    // 数据传输
    void read_from_device();
    void write_to_device();
    void read_from_device_1d/2d/3d();
    void write_to_device_1d/2d/3d();
    
    // 缓冲区管理
    void add_host_buffer();
    void add_device_buffer();
    void delete_host_buffer();
    void delete_device_buffer();
};
```

**零拷贝优化**：
- 自动检测 CPU/iGPU 环境
- 使用 `CL_MEM_USE_HOST_PTR` 避免数据复制

### Kernel 类

```cpp
class Kernel {
public:
    // 构造函数
    template<class... T>
    Kernel(const Device& device, const ulong N, const string& name,
           const T&... parameters);
    
    // 执行控制
    Kernel& run(const uint t=1u);
    Kernel& enqueue_run(const uint t=1u);
    Kernel& finish_queue();
    
    // 参数管理
    template<class... T>
    Kernel& add_parameters(const T&... parameters);
};
```

---

## 依赖关系

### 外部依赖

```
┌─────────────────────────────────────────────────────────────┐
│                    External Dependencies                     │
├─────────────────────────────────────────────────────────────┤
│  OpenCL Headers (KhronosGroup)                               │
│  - CL/cl.h                                                    │
│  - CL/opencl.hpp (C++ bindings)                               │
├─────────────────────────────────────────────────────────────┤
│  C++ Standard Library                                         │
│  - <cmath>, <vector>, <string>                                │
│  - <iostream>, <chrono>, <thread>                             │
│  - <regex>, <filesystem> (C++17)                              │
├─────────────────────────────────────────────────────────────┤
│  System Libraries                                             │
│  - libOpenCL.so (Linux)                                       │
│  - OpenCL.framework (macOS)                                   │
│  - OpenCL.dll (Windows)                                       │
└─────────────────────────────────────────────────────────────┘
```

### 内部模块依赖图

```
main.cpp
    │
    ├──► opencl.hpp
    │        │
    │        ├──► utilities.hpp
    │        │
    │        └──► CL/opencl.hpp (外部)
    │
    └──► kernel.hpp
             │
             ├──► utilities.hpp
             │
             └──► kernel.cpp
```

### 编译依赖

| 平台 | 编译器 | 链接库 |
|------|--------|--------|
| Linux | g++ | -lOpenCL, -pthread |
| macOS | clang++ | -framework OpenCL |
| Windows | MSVC | OpenCL.lib |
| Android | g++ (交叉编译) | -lOpenCL, -ldl |

---

## 项目运行方式

### 编译方式

#### 方式一：使用原始脚本
```bash
chmod +x make.sh
./make.sh [device_id...]
```

#### 方式二：跨平台编译
```bash
chmod +x build_all.sh
./build_all.sh
```

### 运行方式

#### 基本运行
```bash
# 测试所有设备
./OpenCL-Benchmark-linux-x86_64

# 测试指定设备
./OpenCL-Benchmark-linux-x86_64 0 1 2
```

#### Android 运行
```bash
# 推送到设备
adb push OpenCL-Benchmark-android-arm64 /data/local/tmp/
adb shell chmod +x /data/local/tmp/OpenCL-Benchmark-android-arm64
adb shell /data/local/tmp/OpenCL-Benchmark-android-arm64
```

### 测试流程

```
1. 初始化 OpenCL 平台和设备
        │
        ▼
2. 枚举所有可用设备
        │
        ▼
3. 对每个设备执行测试:
   ├── 编译 OpenCL 内核
   ├── 分配内存缓冲区
   ├── 执行计算性能测试
   │   ├── FP64 (double)
   │   ├── FP32 (float)
   │   ├── FP16 (half)
   │   ├── INT64 (long)
   │   ├── INT32 (int)
   │   ├── INT16 (short)
   │   └── INT8 (char/dp4a)
   ├── 执行内存带宽测试
   │   ├── 合并读写
   │   └── 非对齐读写
   └── 执行 PCIe 带宽测试 (仅GPU)
       ├── 发送带宽
       ├── 接收带宽
       └── 双向带宽
        │
        ▼
4. 输出格式化结果
```

---

## 性能指标说明

### 计算性能指标

| 指标 | 单位 | 说明 |
|------|------|------|
| FP64 TFLOPs/s | 万亿次/秒 | 64位浮点运算性能 |
| FP32 TFLOPs/s | 万亿次/秒 | 32位浮点运算性能 |
| FP16 TFLOPs/s | 万亿次/秒 | 16位浮点运算性能 |
| INT64 TIOPs/s | 万亿次/秒 | 64位整数运算性能 |
| INT32 TIOPs/s | 万亿次/秒 | 32位整数运算性能 |
| INT16 TIOPs/s | 万亿次/秒 | 16位整数运算性能 |
| INT8 TIOPs/s | 万亿次/秒 | 8位整数运算性能 |

### 内存带宽指标

| 指标 | 说明 |
|------|------|
| Coalesced Read | 合并读取模式带宽 |
| Coalesced Write | 合并写入模式带宽 |
| Misaligned Read | 非对齐读取模式带宽 |
| Misaligned Write | 非对齐写入模式带宽 |

### PCIe 带宽指标

| 指标 | 说明 |
|------|------|
| Send | 主机到设备传输带宽 |
| Receive | 设备到主机传输带宽 |
| Bidirectional | 双向传输带宽 |

---

## 扩展和定制

### 添加新的测试内核

1. 在 `kernel.cpp` 中添加新的内核函数
2. 在 `main.cpp` 中添加测试调用逻辑
3. 重新编译项目

### 支持新平台

1. 在 `build_all.sh` 中添加新的编译目标
2. 配置相应的交叉编译工具链
3. 处理平台特定的 OpenCL 库路径

### 自定义测试参数

在 `main.cpp` 中可调整：
- `memory_allocation_size` - 内存分配大小
- `N_kernel` - 内核迭代次数
- `N_memory` - 内存传输迭代次数

---

## 版本信息

- **项目版本**: 基于 OpenCL-Benchmark 原版
- **OpenCL 版本**: 支持 1.0 - 3.0
- **C++ 标准**: C++17
- **支持平台**: Windows, Linux, macOS, Android
- **支持架构**: x86_64, ARM32, ARM64

---

## 许可证

本项目遵循原 OpenCL-Benchmark 项目的许可证。
