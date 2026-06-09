// Standalone OpenCL Loader and Comprehensive Benchmark
// Does not depend on any system OpenCL headers
// Compiles with: g++ -std=c++17 -O2 standalone_benchmark.cpp -o benchmark -ldl

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <iomanip>
#include <algorithm>
#include <dlfcn.h>

// ============================================================
// Minimal OpenCL Type Definitions (no headers needed)
// ============================================================
typedef int32_t  cl_int;
typedef uint32_t cl_uint;
typedef int64_t  cl_long;
typedef uint64_t cl_ulong;
typedef int32_t  cl_bool;
typedef uint32_t cl_bitfield;
typedef cl_bitfield cl_device_type;
typedef cl_bitfield cl_platform_info;
typedef cl_bitfield cl_device_info;
typedef cl_bitfield cl_command_queue_properties;
typedef cl_bitfield cl_mem_flags;
typedef void* cl_context_properties;

typedef struct _cl_platform_id*    cl_platform_id;
typedef struct _cl_device_id*      cl_device_id;
typedef struct _cl_context*        cl_context;
typedef struct _cl_command_queue*  cl_command_queue;
typedef struct _cl_mem*            cl_mem;
typedef struct _cl_program*        cl_program;
typedef struct _cl_kernel*         cl_kernel;
typedef struct _cl_event*          cl_event;

// Error codes
static const int CL_SUCCESS = 0;

// Bool values
static const cl_bool CL_FALSE = 0;
static const cl_bool CL_TRUE = 1;

// Device types
static const cl_bitfield CL_DEVICE_TYPE_GPU = (1 << 2);
static const cl_bitfield CL_DEVICE_TYPE_CPU = (1 << 1);
static const cl_bitfield CL_DEVICE_TYPE_ALL = 0xFFFFFFFF;

// Memory flags
static const cl_bitfield CL_MEM_READ_WRITE = (1 << 0);
static const cl_bitfield CL_MEM_WRITE_ONLY = (1 << 1);
static const cl_bitfield CL_MEM_READ_ONLY = (1 << 2);
static const cl_bitfield CL_MEM_COPY_HOST_PTR = (1 << 5);

// Platform/Device info constants
static const cl_platform_info CL_PLATFORM_NAME = 0x0902;
static const cl_platform_info CL_PLATFORM_VENDOR = 0x0903;
static const cl_platform_info CL_PLATFORM_VERSION = 0x0904;
static const cl_platform_info CL_PLATFORM_EXTENSIONS = 0x0905;
static const cl_device_info CL_DEVICE_NAME = 0x102B;
static const cl_device_info CL_DEVICE_VENDOR = 0x102C;
static const cl_device_info CL_DEVICE_VERSION = 0x102D;
static const cl_device_info CL_DEVICE_EXTENSIONS = 0x1030;
static const cl_device_info CL_DEVICE_TYPE = 0x1000;
static const cl_device_info CL_DEVICE_MAX_COMPUTE_UNITS = 0x1002;
static const cl_device_info CL_DEVICE_MAX_CLOCK_FREQUENCY = 0x100C;
static const cl_device_info CL_DEVICE_GLOBAL_MEM_SIZE = 0x101F;
static const cl_device_info CL_DEVICE_LOCAL_MEM_SIZE = 0x1023;
static const cl_device_info CL_DEVICE_MAX_WORK_GROUP_SIZE = 0x1004;
static const cl_device_info CL_DEVICE_OPENCL_C_VERSION = 0x103D;
static const cl_device_info CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT = 0x103A;
static const cl_device_info CL_DEVICE_PROFILING_TIMER_RESOLUTION = 0x1028;

// Build info
static const cl_uint CL_PROGRAM_BUILD_LOG = 0x1183;

// ============================================================
// Dynamic Function Loader
// ============================================================
class OpenCLDynamic {
public:
    void* handle = nullptr;

    typedef int (*fn_clGetPlatformIDs)(cl_uint, cl_platform_id*, cl_uint*);
    typedef int (*fn_clGetPlatformInfo)(cl_platform_id, cl_platform_info, size_t, void*, size_t*);
    typedef int (*fn_clGetDeviceIDs)(cl_platform_id, cl_device_type, cl_uint, cl_device_id*, cl_uint*);
    typedef int (*fn_clGetDeviceInfo)(cl_device_id, cl_device_info, size_t, void*, size_t*);
    typedef cl_context (*fn_clCreateContext)(const cl_context_properties*, cl_uint, const cl_device_id*,
                                         void(*)(const char*, const void*, size_t, void*),
                                         void*, int*);
    typedef int (*fn_clReleaseContext)(cl_context);
    typedef cl_command_queue (*fn_clCreateCommandQueue)(cl_context, cl_device_id, cl_command_queue_properties, int*);
    typedef int (*fn_clReleaseCommandQueue)(cl_command_queue);
    typedef int (*fn_clFinish)(cl_command_queue);
    typedef cl_mem (*fn_clCreateBuffer)(cl_context, cl_mem_flags, size_t, void*, int*);
    typedef int (*fn_clReleaseMemObject)(cl_mem);
    typedef int (*fn_clEnqueueReadBuffer)(cl_command_queue, cl_mem, cl_bool, size_t, size_t, void*,
                                          cl_uint, const cl_event*, cl_event*);
    typedef int (*fn_clEnqueueWriteBuffer)(cl_command_queue, cl_mem, cl_bool, size_t, size_t, const void*,
                                           cl_uint, const cl_event*, cl_event*);
    typedef cl_program (*fn_clCreateProgramWithSource)(cl_context, cl_uint, const char**, const size_t*, int*);
    typedef int (*fn_clReleaseProgram)(cl_program);
    typedef int (*fn_clBuildProgram)(cl_program, cl_uint, const cl_device_id*, const char*,
                                      void(*)(cl_program, void*), void*);
    typedef int (*fn_clGetProgramBuildInfo)(cl_program, cl_device_id, cl_uint, size_t, void*, size_t*);
    typedef cl_kernel (*fn_clCreateKernel)(cl_program, const char*, int*);
    typedef int (*fn_clReleaseKernel)(cl_kernel);
    typedef int (*fn_clSetKernelArg)(cl_kernel, cl_uint, size_t, const void*);
    typedef int (*fn_clEnqueueNDRangeKernel)(cl_command_queue, cl_kernel, cl_uint, const size_t*,
                                             const size_t*, const size_t*,
                                             cl_uint, const cl_event*, cl_event*);

    fn_clGetPlatformIDs clGetPlatformIDs = nullptr;
    fn_clGetPlatformInfo clGetPlatformInfo = nullptr;
    fn_clGetDeviceIDs clGetDeviceIDs = nullptr;
    fn_clGetDeviceInfo clGetDeviceInfo = nullptr;
    fn_clCreateContext clCreateContext = nullptr;
    fn_clReleaseContext clReleaseContext = nullptr;
    fn_clCreateCommandQueue clCreateCommandQueue = nullptr;
    fn_clReleaseCommandQueue clReleaseCommandQueue = nullptr;
    fn_clFinish clFinish = nullptr;
    fn_clCreateBuffer clCreateBuffer = nullptr;
    fn_clReleaseMemObject clReleaseMemObject = nullptr;
    fn_clEnqueueReadBuffer clEnqueueReadBuffer = nullptr;
    fn_clEnqueueWriteBuffer clEnqueueWriteBuffer = nullptr;
    fn_clCreateProgramWithSource clCreateProgramWithSource = nullptr;
    fn_clReleaseProgram clReleaseProgram = nullptr;
    fn_clBuildProgram clBuildProgram = nullptr;
    fn_clGetProgramBuildInfo clGetProgramBuildInfo = nullptr;
    fn_clCreateKernel clCreateKernel = nullptr;
    fn_clReleaseKernel clReleaseKernel = nullptr;
    fn_clSetKernelArg clSetKernelArg = nullptr;
    fn_clEnqueueNDRangeKernel clEnqueueNDRangeKernel = nullptr;

    bool load() {
        const char* lib_names[] = {
            "libOpenCL.so", "libOpenCL.so.1", "libmali.so", "libMali.so",
            "libGLES_mali.so", "libGLESv2_mali.so", "libPOCL.so", "libpocl.so"
        };

        std::cout << "Searching for OpenCL library..." << std::endl;
        for (const char* name : lib_names) {
            handle = dlopen(name, RTLD_NOW | RTLD_GLOBAL);
            if (handle) {
                std::cout << "  Loaded: " << name << std::endl;
                break;
            }
        }

        if (!handle) {
            std::cerr << "  Error: " << dlerror() << std::endl;
            return false;
        }

        #define LOAD_FN(name) \
            name = (fn_##name)dlsym(handle, #name); \
            if (!name) { std::cerr << "  Warning: " << #name << " not found" << std::endl; }

        LOAD_FN(clGetPlatformIDs);
        LOAD_FN(clGetPlatformInfo);
        LOAD_FN(clGetDeviceIDs);
        LOAD_FN(clGetDeviceInfo);
        LOAD_FN(clCreateContext);
        LOAD_FN(clReleaseContext);
        LOAD_FN(clCreateCommandQueue);
        LOAD_FN(clReleaseCommandQueue);
        LOAD_FN(clFinish);
        LOAD_FN(clCreateBuffer);
        LOAD_FN(clReleaseMemObject);
        LOAD_FN(clEnqueueReadBuffer);
        LOAD_FN(clEnqueueWriteBuffer);
        LOAD_FN(clCreateProgramWithSource);
        LOAD_FN(clReleaseProgram);
        LOAD_FN(clBuildProgram);
        LOAD_FN(clGetProgramBuildInfo);
        LOAD_FN(clCreateKernel);
        LOAD_FN(clReleaseKernel);
        LOAD_FN(clSetKernelArg);
        LOAD_FN(clEnqueueNDRangeKernel);

        #undef LOAD_FN

        return clGetPlatformIDs != nullptr;
    }

    ~OpenCLDynamic() {
        if (handle) dlclose(handle);
    }
};

static OpenCLDynamic g_ocl;

// ============================================================
// OpenCL Kernel Code (Comprehensive Benchmark)
// ============================================================
static const char* kernel_code = R"(
#define def_N 262144u
#define def_M 16u

// FP64 (double precision) - requires cl_khr_fp64
#ifdef cl_khr_fp64
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
kernel void kernel_double(global float* data) {
    double x = (double)get_global_id(0);
    double y = (double)get_local_id(0);
    for(uint i=0u; i<128u; i++) {
        x = fma(y, x, y);
        y = fma(x, y, x);
    }
    data[get_global_id(0)] = (float)y;
}
#endif

// FP32 (float precision)
kernel void kernel_float(global float* data) {
    float x = (float)get_global_id(0);
    float y = (float)get_local_id(0);
    for(uint i=0u; i<512u; i++) {
        x = fma(y, x, y);
        y = fma(x, y, x);
    }
    data[get_global_id(0)] = y;
}

// FP16 (half precision) - requires cl_khr_fp16
#ifdef cl_khr_fp16
#pragma OPENCL EXTENSION cl_khr_fp16 : enable
kernel void kernel_half(global float* data) {
    half2 x = (half2)((float)get_global_id(0), (float)get_local_id(0));
    half2 y = (half2)((float)get_local_id(0), (float)get_global_id(0));
    for(uint i=0u; i<512u; i++) {
        x = y*x+y;
        y = x*y+x;
    }
    data[get_global_id(0)] = (float)y.x+(float)y.y;
}
#endif

// INT64
kernel void kernel_long(global float* data) {
    long x = (long)get_global_id(0);
    long y = (long)get_local_id(0);
    for(uint i=0u; i<8u; i++) {
        x = y*x+y;
        y = x*y+x;
    }
    data[get_global_id(0)] = as_float((int)y);
}

// INT32
kernel void kernel_int(global float* data) {
    int x = get_global_id(0);
    int y = get_local_id(0);
    for(uint i=0u; i<512u; i++) {
        x = y*x+y;
        y = x*y+x;
    }
    data[get_global_id(0)] = as_float(y);
}

// INT16
kernel void kernel_short(global float* data) {
    short2 x = as_short2((uint)get_global_id(0));
    short2 y = as_short2((uint)get_local_id(0));
    for(uint i=0u; i<128u; i++) {
        x = y*x+y;
        y = x*y+x;
    }
    data[get_global_id(0)] = as_float(y);
}

// INT8 (using dp4a if available)
kernel void kernel_char(global float* data) {
    char4 x = as_char4((uint)get_global_id(0));
    char4 y = as_char4((uint)get_local_id(0));
    for(uint i=0u; i<64u; i++) {
        int tmp_x = x.x*y.x + x.y*y.y + x.z*y.z + x.w*y.w + (int)y.x;
        int tmp_y = y.x*x.x + y.y*x.y + y.z*x.z + y.w*x.w + (int)x.x;
        x = as_char4(tmp_x);
        y = as_char4(tmp_y);
    }
    data[get_global_id(0)] = as_float(y);
}

// Memory bandwidth tests
kernel void kernel_coalesced_write(global float* data) {
    const uint n = get_global_id(0);
    for(uint i=0u; i<def_M; i++) data[i*def_N+n] = as_float(n);
}

kernel void kernel_coalesced_read(global float* data) {
    const uint n = get_global_id(0);
    float x = 0.0f;
    for(uint i=0u; i<def_M; i++) x += data[i*def_N+n];
    data[n] = x;
}

kernel void kernel_misaligned_write(global float* data) {
    const uint n = get_global_id(0);
    for(uint i=0u; i<def_M; i++) data[n*def_M+i] = as_float(n);
}

kernel void kernel_misaligned_read(global float* data) {
    const uint n = get_global_id(0);
    float x = 0.0f;
    for(uint i=0u; i<def_M; i++) x += data[n*def_M+i];
    data[n] = x;
}
)";

// ============================================================
// Helper Functions
// ============================================================
std::string getInfoString(cl_platform_id platform, cl_platform_info param) {
    char buf[1024] = {0};
    if (g_ocl.clGetPlatformInfo) {
        g_ocl.clGetPlatformInfo(platform, param, sizeof(buf), buf, nullptr);
    }
    return std::string(buf);
}

std::string getDeviceString(cl_device_id device, cl_device_info param) {
    char buf[1024] = {0};
    if (g_ocl.clGetDeviceInfo) {
        g_ocl.clGetDeviceInfo(device, param, sizeof(buf), buf, nullptr);
    }
    return std::string(buf);
}

cl_ulong getDeviceULong(cl_device_id device, cl_device_info param) {
    cl_ulong val = 0;
    if (g_ocl.clGetDeviceInfo) {
        g_ocl.clGetDeviceInfo(device, param, sizeof(val), &val, nullptr);
    }
    return val;
}

cl_uint getDeviceUInt(cl_device_id device, cl_device_info param) {
    cl_uint val = 0;
    if (g_ocl.clGetDeviceInfo) {
        g_ocl.clGetDeviceInfo(device, param, sizeof(val), &val, nullptr);
    }
    return val;
}

size_t getDeviceSizeT(cl_device_id device, cl_device_info param) {
    size_t val = 0;
    if (g_ocl.clGetDeviceInfo) {
        g_ocl.clGetDeviceInfo(device, param, sizeof(val), &val, nullptr);
    }
    return val;
}

bool hasExtension(const std::string& extensions, const std::string& ext) {
    return extensions.find(ext) != std::string::npos;
}

std::string fraction(float x) {
    float values[] = {1.0f/64, 1.0f/32, 1.0f/24, 1.0f/16, 1.0f/12, 1.0f/8, 1.0f/4, 1.0f/3, 1.0f/2, 2.0f/3, 1, 2, 4, 8, 16, 32, 64};
    std::string strs[] = {"1/64", "1/32", "1/24", "1/16", "1/12", "1/8 ", "1/4 ", "1/3 ", "1/2 ", "2/3 ", " 1x ", " 2x ", " 4x ", " 8x ", " 16x", " 32x", " 64x"};
    int imin = 0;
    float vmin = 1e30f;
    for (int i = 0; i < 17; i++) {
        float vnew = std::pow(0.01f*x - values[i], 2);
        if (vnew <= vmin) { vmin = vnew; imin = i; }
    }
    return "(" + strs[imin] + ")";
}

std::string alignr(int width, const std::string& s) {
    if ((int)s.length() >= width) return s;
    return std::string(width - s.length(), ' ') + s;
}

std::string formatFloat(float val, int prec) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(prec) << val;
    return oss.str();
}

// ============================================================
// Main
// ============================================================
int main() {
    std::cout << ".-----------------------------------------------------------------------------." << std::endl;
    std::cout << "|                       OpenCL-Benchmark (Standalone)                         |" << std::endl;
    std::cout << "'-----------------------------------------------------------------------------'" << std::endl;
    std::cout << std::endl;

    if (!g_ocl.load()) {
        std::cerr << "Failed to load OpenCL!" << std::endl;
        return 1;
    }

    // Get platforms
    cl_uint num_platforms = 0;
    g_ocl.clGetPlatformIDs(0, nullptr, &num_platforms);
    if (num_platforms == 0) {
        std::cerr << "No OpenCL platforms found!" << std::endl;
        return 1;
    }

    std::cout << "Found " << num_platforms << " platform(s)" << std::endl;
    std::vector<cl_platform_id> platforms(num_platforms);
    g_ocl.clGetPlatformIDs(num_platforms, platforms.data(), nullptr);

    // Find GPU device
    cl_device_id gpu_device = nullptr;
    cl_platform_id gpu_platform = nullptr;

    for (cl_uint p = 0; p < num_platforms; p++) {
        cl_uint num_devs = 0;
        g_ocl.clGetDeviceIDs(platforms[p], CL_DEVICE_TYPE_GPU, 0, nullptr, &num_devs);
        if (num_devs > 0) {
            g_ocl.clGetDeviceIDs(platforms[p], CL_DEVICE_TYPE_GPU, 1, &gpu_device, nullptr);
            gpu_platform = platforms[p];
            break;
        }
    }

    if (!gpu_device) {
        // Fallback to CPU
        for (cl_uint p = 0; p < num_platforms; p++) {
            cl_uint num_devs = 0;
            g_ocl.clGetDeviceIDs(platforms[p], CL_DEVICE_TYPE_CPU, 0, nullptr, &num_devs);
            if (num_devs > 0) {
                g_ocl.clGetDeviceIDs(platforms[p], CL_DEVICE_TYPE_CPU, 1, &gpu_device, nullptr);
                gpu_platform = platforms[p];
                break;
            }
        }
    }

    if (!gpu_device) {
        std::cerr << "No OpenCL devices found!" << std::endl;
        return 1;
    }

    // Get device info
    std::string dev_name = getDeviceString(gpu_device, CL_DEVICE_NAME);
    std::string dev_vendor = getDeviceString(gpu_device, CL_DEVICE_VENDOR);
    std::string dev_version = getDeviceString(gpu_device, CL_DEVICE_VERSION);
    std::string dev_extensions = getDeviceString(gpu_device, CL_DEVICE_EXTENSIONS);
    cl_uint compute_units = getDeviceUInt(gpu_device, CL_DEVICE_MAX_COMPUTE_UNITS);
    cl_uint max_freq = getDeviceUInt(gpu_device, CL_DEVICE_MAX_CLOCK_FREQUENCY);
    cl_ulong global_mem = getDeviceULong(gpu_device, CL_DEVICE_GLOBAL_MEM_SIZE);
    size_t max_wg_size = getDeviceSizeT(gpu_device, CL_DEVICE_MAX_WORK_GROUP_SIZE);

    // Check FP64/FP16 support
    bool has_fp64 = hasExtension(dev_extensions, "cl_khr_fp64") || hasExtension(dev_extensions, "cl_amd_fp64");
    bool has_fp16 = hasExtension(dev_extensions, "cl_khr_fp16") || hasExtension(dev_extensions, "cl_amd_fp16");

    // Estimate peak performance (very rough)
    float estimated_tflops = (float)compute_units * (float)max_freq * 0.001f * 0.064f; // rough estimate

    std::cout << std::endl;
    std::cout << ".-----------------------------------------------------------------------------." << std::endl;
    std::cout << "| Device: " << std::left << std::setw(68) << dev_name << "|" << std::endl;
    std::cout << "| Vendor: " << std::left << std::setw(68) << dev_vendor << "|" << std::endl;
    std::cout << "| Version: " << std::left << std::setw(67) << dev_version << "|" << std::endl;
    std::cout << "| Compute Units: " << std::left << std::setw(60) << compute_units << "|" << std::endl;
    std::cout << "| Max Frequency: " << std::left << std::setw(60) << (std::to_string(max_freq) + " MHz") << "|" << std::endl;
    std::cout << "| Global Memory: " << std::left << std::setw(60) << (std::to_string(global_mem / 1024 / 1024) + " MB") << "|" << std::endl;
    std::cout << "| Max Work Group Size: " << std::left << std::setw(53) << max_wg_size << "|" << std::endl;
    std::cout << "| FP64 Support: " << std::left << std::setw(61) << (has_fp64 ? "Yes" : "No") << "|" << std::endl;
    std::cout << "| FP16 Support: " << std::left << std::setw(61) << (has_fp16 ? "Yes" : "No") << "|" << std::endl;
    std::cout << "'-----------------------------------------------------------------------------'" << std::endl;
    std::cout << std::endl;

    // Create context
    cl_int ctx_err = 0;
    cl_context context = g_ocl.clCreateContext(nullptr, 1, &gpu_device, nullptr, nullptr, &ctx_err);
    if (ctx_err != CL_SUCCESS) {
        std::cerr << "Failed to create context: " << ctx_err << std::endl;
        return 1;
    }

    // Create command queue
    cl_int q_err = 0;
    cl_command_queue queue = g_ocl.clCreateCommandQueue(context, gpu_device, 0, &q_err);
    if (q_err != CL_SUCCESS) {
        std::cerr << "Failed to create command queue: " << q_err << std::endl;
        g_ocl.clReleaseContext(context);
        return 1;
    }

    // Create and build program
    cl_program program = g_ocl.clCreateProgramWithSource(context, 1, &kernel_code, nullptr, nullptr);
    cl_int err = g_ocl.clBuildProgram(program, 1, &gpu_device, nullptr, nullptr, nullptr);
    if (err != CL_SUCCESS) {
        char log[8192] = {0};
        g_ocl.clGetProgramBuildInfo(program, gpu_device, CL_PROGRAM_BUILD_LOG, sizeof(log), log, nullptr);
        std::cerr << "Build failed: " << err << std::endl;
        std::cerr << "Log: " << log << std::endl;
        g_ocl.clReleaseProgram(program);
        g_ocl.clReleaseCommandQueue(queue);
        g_ocl.clReleaseContext(context);
        return 1;
    }

    std::cout << ".-----------------------------------------------------------------------------." << std::endl;
    std::cout << "| Info: OpenCL C code successfully compiled.                                  |" << std::endl;

    // Benchmark parameters
    const size_t N = 262144; // kernel range
    const size_t M = 16;     // coalescence size
    const int N_kernel = 16; // iterations

    // Allocate buffer
    std::vector<float> data(N * M, 0.0f);
    cl_mem buffer = g_ocl.clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                         N * M * sizeof(float), data.data(), nullptr);

    auto benchmark_kernel = [&](const char* name, int ops_per_iter, int inner_iters) -> double {
        cl_kernel kernel = g_ocl.clCreateKernel(program, name, nullptr);
        if (!kernel) {
            std::cerr << "  Warning: kernel '" << name << "' not found" << std::endl;
            return 1e30;
        }

        g_ocl.clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer);

        size_t global_size = N;
        size_t local_size = 256;

        // Warmup
        cl_int err = g_ocl.clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &global_size, &local_size, 0, nullptr, nullptr);
        if (err != CL_SUCCESS) {
            std::cerr << "  Error: kernel '" << name << "' warmup failed: " << err << std::endl;
            g_ocl.clReleaseKernel(kernel);
            return 1e30;
        }
        g_ocl.clFinish(queue);

        // Benchmark - run multiple times and sum for better precision
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < N_kernel; i++) {
            g_ocl.clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &global_size, &local_size, 0, nullptr, nullptr);
        }
        g_ocl.clFinish(queue);
        auto end = std::chrono::high_resolution_clock::now();
        
        double total_time_ms = std::chrono::duration<double, std::milli>(end - start).count();
        double avg_time_ms = total_time_ms / N_kernel;

        g_ocl.clReleaseKernel(kernel);
        return avg_time_ms;
    };

    // FP64 Compute
    if (has_fp64) {
        double time_ms = benchmark_kernel("kernel_double", 512, 128);
        if (time_ms < 1e20 && time_ms > 0) {
            // TFLOPS = ops_per_iter * N / time_seconds / 1e12
            // time_ms is in milliseconds, so: TFLOPS = ops_per_iter * N / (time_ms/1000) / 1e12
            float tflops = 512.0f * (float)N / (float)time_ms * 1e-9f;
            std::cout << "| FP64  compute                                          " 
                      << alignr(15, formatFloat(tflops, 3)) << " TFLOPs/s " << fraction(100.0f*tflops/estimated_tflops) << " |" << std::endl;
        }
    } else {
        std::cout << "| FP64  compute                                          not supported        |" << std::endl;
    }

    // FP32 Compute
    {
        double time_ms = benchmark_kernel("kernel_float", 2048, 512);
        if (time_ms < 1e20 && time_ms > 0) {
            float tflops = 2048.0f * (float)N / (float)time_ms * 1e-9f;
            std::cout << "| FP32  compute                                          " 
                      << alignr(15, formatFloat(tflops, 3)) << " TFLOPs/s " << fraction(100.0f*tflops/estimated_tflops) << " |" << std::endl;
        }
    }

    // FP16 Compute
    if (has_fp16) {
        double time_ms = benchmark_kernel("kernel_half", 4096, 512);
        if (time_ms < 1e20 && time_ms > 0) {
            float tflops = 4096.0f * (float)N / (float)time_ms * 1e-9f;
            std::cout << "| FP16  compute                                          " 
                      << alignr(15, formatFloat(tflops, 3)) << " TFLOPs/s " << fraction(100.0f*tflops/estimated_tflops) << " |" << std::endl;
        }
    } else {
        std::cout << "| FP16  compute                                          not supported        |" << std::endl;
    }

    // INT64 Compute
    {
        double time_ms = benchmark_kernel("kernel_long", 32, 8);
        if (time_ms < 1e20 && time_ms > 0) {
            float tiops = 32.0f * (float)N / (float)time_ms * 1e-9f;
            std::cout << "| INT64 compute                                          " 
                      << alignr(15, formatFloat(tiops, 3)) << "  TIOPs/s " << fraction(100.0f*tiops/estimated_tflops) << " |" << std::endl;
        }
    }

    // INT32 Compute
    {
        double time_ms = benchmark_kernel("kernel_int", 2048, 512);
        if (time_ms < 1e20 && time_ms > 0) {
            float tiops = 2048.0f * (float)N / (float)time_ms * 1e-9f;
            std::cout << "| INT32 compute                                          " 
                      << alignr(15, formatFloat(tiops, 3)) << "  TIOPs/s " << fraction(100.0f*tiops/estimated_tflops) << " |" << std::endl;
        }
    }

    // INT16 Compute
    {
        double time_ms = benchmark_kernel("kernel_short", 1024, 128);
        if (time_ms < 1e20 && time_ms > 0) {
            float tiops = 1024.0f * (float)N / (float)time_ms * 1e-9f;
            std::cout << "| INT16 compute                                          " 
                      << alignr(15, formatFloat(tiops, 4)) << "  TIOPs/s " << fraction(100.0f*tiops/estimated_tflops) << " |" << std::endl;
        }
    }

    // INT8 Compute
    {
        double time_ms = benchmark_kernel("kernel_char", 1024, 64);
        if (time_ms < 1e20 && time_ms > 0) {
            float tiops = 1024.0f * (float)N / (float)time_ms * 1e-9f;
            std::cout << "| INT8  compute                                          " 
                      << alignr(15, formatFloat(tiops, 4)) << "  TIOPs/s " << fraction(100.0f*tiops/estimated_tflops) << " |" << std::endl;
        }
    }

    // Memory Bandwidth - Coalesced Write
    {
        cl_kernel kernel = g_ocl.clCreateKernel(program, "kernel_coalesced_write", nullptr);
        g_ocl.clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer);
        size_t global = N, local = 256;
        
        g_ocl.clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &global, &local, 0, nullptr, nullptr);
        g_ocl.clFinish(queue);
        
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < N_kernel; i++) {
            g_ocl.clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &global, &local, 0, nullptr, nullptr);
        }
        g_ocl.clFinish(queue);
        auto end = std::chrono::high_resolution_clock::now();
        double total_time_ms = std::chrono::duration<double, std::milli>(end - start).count();
        double avg_time_ms = total_time_ms / N_kernel;
        // BW = bytes / time_seconds / 1e9 = 4 * N * M / (avg_time_ms/1000) / 1e9 = 4 * N * M / avg_time_ms * 1e-6
        float bw = 4.0f * N * M / (float)avg_time_ms * 1e-6f;
        std::cout << "| Memory Bandwidth (coalesced write)                     " 
                  << alignr(18, formatFloat(bw, 2)) << " GB/s |" << std::endl;
        g_ocl.clReleaseKernel(kernel);
    }

    // Memory Bandwidth - Coalesced Read
    {
        cl_kernel kernel = g_ocl.clCreateKernel(program, "kernel_coalesced_read", nullptr);
        g_ocl.clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer);
        size_t global = N, local = 256;
        
        g_ocl.clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &global, &local, 0, nullptr, nullptr);
        g_ocl.clFinish(queue);
        
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < N_kernel; i++) {
            g_ocl.clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &global, &local, 0, nullptr, nullptr);
        }
        g_ocl.clFinish(queue);
        auto end = std::chrono::high_resolution_clock::now();
        double total_time_ms = std::chrono::duration<double, std::milli>(end - start).count();
        double avg_time_ms = total_time_ms / N_kernel;
        float bw = 4.0f * N * M / (float)avg_time_ms * 1e-6f;
        std::cout << "| Memory Bandwidth (coalesced read )                     " 
                  << alignr(18, formatFloat(bw, 2)) << " GB/s |" << std::endl;
        g_ocl.clReleaseKernel(kernel);
    }

    // Memory Bandwidth - Misaligned Write
    {
        cl_kernel kernel = g_ocl.clCreateKernel(program, "kernel_misaligned_write", nullptr);
        g_ocl.clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer);
        size_t global = N, local = 256;
        
        g_ocl.clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &global, &local, 0, nullptr, nullptr);
        g_ocl.clFinish(queue);
        
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < N_kernel; i++) {
            g_ocl.clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &global, &local, 0, nullptr, nullptr);
        }
        g_ocl.clFinish(queue);
        auto end = std::chrono::high_resolution_clock::now();
        double total_time_ms = std::chrono::duration<double, std::milli>(end - start).count();
        double avg_time_ms = total_time_ms / N_kernel;
        float bw = 4.0f * N * M / (float)avg_time_ms * 1e-6f;
        std::cout << "| Memory Bandwidth (misaligned write)                    " 
                  << alignr(18, formatFloat(bw, 2)) << " GB/s |" << std::endl;
        g_ocl.clReleaseKernel(kernel);
    }

    // Memory Bandwidth - Misaligned Read
    {
        cl_kernel kernel = g_ocl.clCreateKernel(program, "kernel_misaligned_read", nullptr);
        g_ocl.clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer);
        size_t global = N, local = 256;
        
        g_ocl.clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &global, &local, 0, nullptr, nullptr);
        g_ocl.clFinish(queue);
        
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < N_kernel; i++) {
            g_ocl.clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &global, &local, 0, nullptr, nullptr);
        }
        g_ocl.clFinish(queue);
        auto end = std::chrono::high_resolution_clock::now();
        double total_time_ms = std::chrono::duration<double, std::milli>(end - start).count();
        double avg_time_ms = total_time_ms / N_kernel;
        float bw = 4.0f * N * M / (float)avg_time_ms * 1e-6f;
        std::cout << "| Memory Bandwidth (misaligned read )                    " 
                  << alignr(18, formatFloat(bw, 2)) << " GB/s |" << std::endl;
        g_ocl.clReleaseKernel(kernel);
    }

    std::cout << "'-----------------------------------------------------------------------------'" << std::endl;

    // Cleanup
    g_ocl.clReleaseMemObject(buffer);
    g_ocl.clReleaseProgram(program);
    g_ocl.clReleaseCommandQueue(queue);
    g_ocl.clReleaseContext(context);

    return 0;
}
