# OpenCL-Benchmark 跨平台編譯說明

## 概述

本項目已成功編譯生成可在以下平台運行的測試程序：
- **Linux x86_64** - 適用於標準Linux桌面/服務器系統
- **Linux ARM32** - 適用於ARM 32位Linux設備（如樹莓派等）
- **Linux ARM64** - 適用於ARM 64位Linux設備
- **Android ARM32** - 適用於Android ARM 32位設備
- **Android ARM64** - 適用於Android ARM 64位設備

## 編譯步驟

### 1. 環境準備

在Ubuntu/Debian系統上安裝交叉編譯工具鏈：

```bash
# 安裝標準編譯工具
sudo apt-get update
sudo apt-get install -y g++ make

# 安裝ARM交叉編譯工具鏈
sudo apt-get install -y gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf
sudo apt-get install -y gcc-aarch64-linux-gnu g++-aarch64-linux-gnu
```

### 2. 執行編譯

```bash
# 克隆項目
git clone https://github.com/kukakong/OpenCL-Benchmark.git
cd OpenCL-Benchmark

# 執行編譯腳本
chmod +x build_all.sh
./build_all.sh
```

### 3. 編譯輸出

編譯完成後，所有可執行文件將位於 `dist/` 目錄下：

| 文件名 | 目標平台 | 說明 |
|--------|----------|------|
| `OpenCL-Benchmark-linux-x86_64` | Linux x86_64 | 標準Linux 64位系統 |
| `OpenCL-Benchmark-linux-arm32` | Linux ARM32 | Linux ARM 32位系統 |
| `OpenCL-Benchmark-linux-arm64` | Linux ARM64 | Linux ARM 64位系統 |
| `OpenCL-Benchmark-android-arm32` | Android ARM32 | Android ARM 32位設備 |
| `OpenCL-Benchmark-android-arm64` | Android ARM64 | Android ARM 64位設備 |

## 使用說明

### Linux 系統使用方法

1. **確認OpenCL驅動已安裝**
   - NVIDIA GPU: 安裝CUDA Toolkit
   - AMD GPU: 安裝ROCm或AMDGPU-PRO驅動
   - Intel GPU: 安裝Intel OpenCL Runtime

2. **運行基準測試**
   ```bash
   # 賦予執行權限
   chmod +x dist/OpenCL-Benchmark-linux-x86_64

   # 運行測試（測試所有設備）
   ./dist/OpenCL-Benchmark-linux-x86_64

   # 運行測試（指定設備ID）
   ./dist/OpenCL-Benchmark-linux-x86_64 0 1
   ```

### Android 系統使用方法

1. **將可執行文件推送到Android設備**
   ```bash
   # 使用adb推送
   adb push dist/OpenCL-Benchmark-android-arm64 /data/local/tmp/

   # 賦予執行權限
   adb shell chmod +x /data/local/tmp/OpenCL-Benchmark-android-arm64

   # 運行測試
   adb shell /data/local/tmp/OpenCL-Benchmark-android-arm64
   ```

2. **注意事項**
   - Android設備需要支持OpenCL
   - 需要有root權限或可執行權限
   - 部分設備可能需要安裝OpenCL驅動

### ARM Linux 系統使用方法

1. **將可執行文件複製到目標設備**
   ```bash
   # 使用scp複製
   scp dist/OpenCL-Benchmark-linux-arm64 user@arm-device:/home/user/

   # 賦予執行權限
   ssh user@arm-device "chmod +x /home/user/OpenCL-Benchmark-linux-arm64"

   # 運行測試
   ssh user@arm-device "/home/user/OpenCL-Benchmark-linux-arm64"
   ```

## 測試項目說明

本基準測試程序會測量以下性能指標：

### 計算性能
- **FP64** (double, fma) - 64位浮點計算性能
- **FP32** (float, fma) - 32位浮點計算性能
- **FP16** (half2, fma) - 16位浮點計算性能
- **INT64** (long, a*b+c) - 64位整數計算性能
- **INT32** (int, a*b+c) - 32位整數計算性能
- **INT16** (short2, a*b+c) - 16位整數計算性能
- **INT8** (char4, dp4a) - 8位整數計算性能

### 內存帶寬
- 合併讀取帶寬 (coalesced read)
- 合併寫入帶寬 (coalesced write)
- 非對齊讀取帶寬 (misaligned read)
- 非對齊寫入帶寬 (misaligned write)

### PCIe帶寬（僅GPU）
- 發送帶寬 (send)
- 接收帶寬 (receive)
- 雙向帶寬 (bidirectional)

## 故障排除

### 常見問題

1. **找不到OpenCL庫**
   ```
   error while loading shared libraries: libOpenCL.so.1: cannot open shared object file
   ```
   解決方法：安裝OpenCL運行時庫
   ```bash
   sudo apt-get install ocl-icd-libopencl1
   ```

2. **沒有可用的OpenCL設備**
   ```
   Error: No OpenCL devices found
   ```
   解決方法：確保已安裝GPU驅動和OpenCL運行時

3. **權限不足（Android）**
   ```
   Permission denied
   ```
   解決方法：確保文件有執行權限，或使用root權限

## 編譯選項說明

### 編譯腳本參數

`build_all.sh` 腳本支持以下編譯選項：

- `-std=c++17` - 使用C++17標準
- `-O3` - 最高優化級別
- `-pthread` - 啟用多線程支持
- `-static-libgcc -static-libstdc++` - 靜態鏈接C++運行時（ARM交叉編譯）

### 交叉編譯說明

對於ARM平台的交叉編譯，我們使用了OpenCL stub庫來解決鏈接問題：
- 編譯時使用stub函數佔位
- 運行時從系統的OpenCL庫加載實際函數

## 版本信息

- 編譯日期：2026-06-04
- 編譯器版本：GCC 13.3.0
- C++標準：C++17
- 目標平台：Linux x86_64, Linux ARM32/ARM64, Android ARM32/ARM64

## 許可證

本項目遵循原OpenCL-Benchmark項目的許可證。
