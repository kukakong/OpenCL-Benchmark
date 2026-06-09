// Standalone OpenCL Loader and Benchmark
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
// Note: cl_int and cl_uint are defined as int32_t/uint32_t per OpenCL spec
typedef int32_t  cl_int;
typedef int8_t   cl_int8;
typedef int16_t  cl_int16;
typedef int32_t  cl_int32;
typedef int64_t  cl_int64;
typedef uint32_t cl_uint;
typedef uint8_t  cl_uint8;
typedef uint16_t cl_uint16;
typedef uint32_t cl_uint32;
typedef uint64_t cl_uint64;
typedef float    cl_float;
typedef double   cl_double;
typedef int64_t  cl_long;
typedef uint64_t cl_ulong;
typedef int32_t  cl_bool;
typedef uint32_t cl_bitfield;
typedef cl_bitfield cl_device_type;
typedef cl_bitfield cl_platform_info;
typedef cl_bitfield cl_device_info;
typedef cl_bitfield cl_context_info;
typedef cl_bitfield cl_command_queue_info;
typedef cl_bitfield cl_mem_info;
typedef cl_bitfield cl_program_info;
typedef cl_bitfield cl_kernel_info;
typedef cl_bitfield cl_event_info;
typedef cl_bitfield cl_build_info;
typedef cl_bitfield cl_command_queue_properties;
typedef cl_bitfield cl_mem_flags;
typedef cl_bitfield cl_map_flags;
typedef void* cl_context_properties;

typedef struct _cl_platform_id*    cl_platform_id;
typedef struct _cl_device_id*      cl_device_id;
typedef struct _cl_context*        cl_context;
typedef struct _cl_command_queue*  cl_command_queue;
typedef struct _cl_mem*            cl_mem;
typedef struct _cl_program*        cl_program;
typedef struct _cl_kernel*         cl_kernel;
typedef struct _cl_event*          cl_event;
typedef struct _cl_sampler*        cl_sampler;

// Error codes
static const int CL_SUCCESS = 0;
static const int CL_DEVICE_NOT_FOUND = -1;
static const int CL_DEVICE_NOT_AVAILABLE = -2;
static const int CL_INVALID_VALUE = -30;
static const int CL_INVALID_DEVICE = -33;
static const int CL_INVALID_CONTEXT = -34;
static const int CL_INVALID_COMMAND_QUEUE = -36;
static const int CL_INVALID_MEM_OBJECT = -38;
static const int CL_INVALID_PROGRAM = -44;
static const int CL_INVALID_KERNEL = -48;
static const int CL_INVALID_ARG_INDEX = -49;
static const int CL_INVALID_WORK_DIMENSION = -53;
static const int CL_OUT_OF_RESOURCES = -5;

// Bool values
static const cl_bool CL_FALSE = 0;
static const cl_bool CL_TRUE = 1;

// Device types
static const cl_bitfield CL_DEVICE_TYPE_DEFAULT = (1 << 0);
static const cl_bitfield CL_DEVICE_TYPE_CPU = (1 << 1);
static const cl_bitfield CL_DEVICE_TYPE_GPU = (1 << 2);
static const cl_bitfield CL_DEVICE_TYPE_ACCELERATOR = (1 << 3);
static const cl_bitfield CL_DEVICE_TYPE_ALL = 0xFFFFFFFF;

// Memory flags
static const cl_bitfield CL_MEM_READ_WRITE = (1 << 0);
static const cl_bitfield CL_MEM_WRITE_ONLY = (1 << 1);
static const cl_bitfield CL_MEM_READ_ONLY = (1 << 2);
static const cl_bitfield CL_MEM_COPY_HOST_PTR = (1 << 5);
static const cl_bitfield CL_MEM_USE_HOST_PTR = (1 << 7);

// Platform info
static const cl_platform_info CL_PLATFORM_NAME = 0x0902;
static const cl_platform_info CL_PLATFORM_VENDOR = 0x0903;
static const cl_platform_info CL_PLATFORM_VERSION = 0x0904;
static const cl_platform_info CL_PLATFORM_EXTENSIONS = 0x0905;

// Device info
static const cl_device_info CL_DEVICE_NAME = 0x102B;
static const cl_device_info CL_DEVICE_VENDOR = 0x102C;
static const cl_device_info CL_DEVICE_VERSION = 0x102D;
static const cl_device_info CL_DRIVER_VERSION = 0x102E;
static const cl_device_info CL_DEVICE_TYPE = 0x1000;
static const cl_device_info CL_DEVICE_MAX_COMPUTE_UNITS = 0x1002;
static const cl_device_info CL_DEVICE_MAX_CLOCK_FREQUENCY = 0x100C;
static const cl_device_info CL_DEVICE_GLOBAL_MEM_SIZE = 0x101F;
static const cl_device_info CL_DEVICE_LOCAL_MEM_SIZE = 0x1023;
static const cl_device_info CL_DEVICE_MAX_WORK_GROUP_SIZE = 0x1004;
static const cl_device_info CL_DEVICE_OPENCL_C_VERSION = 0x103D;

// ============================================================
// Dynamic Function Loader
// ============================================================
class OpenCLDynamic {
public:
    void* handle = nullptr;

    // Function pointer typedefs
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
    typedef int (*fn_clGetProgramBuildInfo)(cl_program, cl_device_id, cl_build_info, size_t, void*, size_t*);
    typedef cl_kernel (*fn_clCreateKernel)(cl_program, const char*, int*);
    typedef int (*fn_clReleaseKernel)(cl_kernel);
    typedef int (*fn_clSetKernelArg)(cl_kernel, cl_uint, size_t, const void*);
    typedef int (*fn_clEnqueueNDRangeKernel)(cl_command_queue, cl_kernel, cl_uint, const size_t*,
                                             const size_t*, const size_t*,
                                             cl_uint, const cl_event*, cl_event*);

    // Function pointers
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
            "libOpenCL.so",
            "libOpenCL.so.1",
            "libmali.so",
            "libMali.so",
            "libGLES_mali.so",
            "libGLESv2_mali.so",
            "libOpenCL.so.1.1",
            "libOpenCL.so.2.0",
            "libOpenCL.so.3.0",
            "libPOCL.so",
            "libpocl.so"
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
// OpenCL Kernel Code
// ============================================================
static const char* kernel_code = R"(
// Test 1: Vector Addition
__kernel void test_add(__global float* a, __global float* b, __global float* c, int n) {
    int i = get_global_id(0);
    if (i < n) c[i] = a[i] + b[i];
}

// Test 2: Compute Benchmark
__kernel void benchmark_compute(__global float* data, int n, int iters) {
    int i = get_global_id(0);
    if (i >= n) return;
    float v = data[i];
    for (int k = 0; k < iters; k++) {
        v = v * 1.01f + 0.5f;
    }
    data[i] = v;
}

// Test 3: Memory Bandwidth (Read/Write)
__kernel void mem_bandwidth(__global float* src, __global float* dst, int n) {
    int i = get_global_id(0);
    if (i < n) {
        dst[i] = src[i];
    }
}

// Test 4: Matrix Multiplication (Naive)
__kernel void matrix_mul(__global float* A, __global float* B, __global float* C, int N) {
    int row = get_global_id(0);
    int col = get_global_id(1);
    if (row < N && col < N) {
        float sum = 0.0f;
        for (int k = 0; k < N; k++) {
            sum += A[row * N + k] * B[k * N + col];
        }
        C[row * N + col] = sum;
    }
}

// Test 5: Local Memory Test
__kernel void local_mem_test(__global float* input, __global float* output, int n) {
    __local float local_data[256];
    int lid = get_local_id(0);
    int gid = get_global_id(0);
    
    if (gid < n) {
        local_data[lid] = input[gid];
        barrier(CLK_LOCAL_MEM_FENCE);
        
        // Simple reduction
        float sum = 0.0f;
        for (int i = 0; i < 256; i++) {
            sum += local_data[i];
        }
        output[gid] = sum / 256.0f;
    }
}

// Test 6: Integer Operations
__kernel void int_operations(__global int* a, __global int* b, __global int* c, int n) {
    int i = get_global_id(0);
    if (i < n) {
        c[i] = a[i] * b[i] + a[i] - b[i];
    }
}

// Test 7: Mixed Precision (Float + Double if supported)
__kernel void mixed_ops(__global float* a, __global float* b, __global float* c, int n) {
    int i = get_global_id(0);
    if (i < n) {
        float v = a[i];
        v = v * v + b[i];
        v = sqrt(v);
        v = sin(v) + cos(v);
        c[i] = v;
    }
}

// Test 8: Atomic Operations
__kernel void atomic_test(__global int* counter, int n) {
    int i = get_global_id(0);
    if (i < n) {
        atomic_inc(counter);
    }
}

// Test 9: Parallel Reduction
__kernel void parallel_reduction(__global float* input, __global float* partial_sums, int n, __local float* temp) {
    int lid = get_local_id(0);
    int gid = get_global_id(0);
    int lsize = get_local_size(0);
    
    temp[lid] = (gid < n) ? input[gid] : 0.0f;
    barrier(CLK_LOCAL_MEM_FENCE);
    
    for (int stride = lsize / 2; stride > 0; stride /= 2) {
        if (lid < stride) {
            temp[lid] += temp[lid + stride];
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }
    
    if (lid == 0) {
        partial_sums[get_group_id(0)] = temp[0];
    }
}

// Test 10: Vector Types
__kernel void vector_types(__global float4* a, __global float4* b, __global float4* c, int n) {
    int i = get_global_id(0);
    if (i < n) {
        c[i] = a[i] + b[i];
    }
}
)";

// ============================================================
// Helper Functions
// ============================================================
std::string getInfoString(cl_platform_id platform, cl_platform_info param) {
    char buf[256] = {0};
    if (g_ocl.clGetPlatformInfo) {
        g_ocl.clGetPlatformInfo(platform, param, sizeof(buf), buf, nullptr);
    }
    return std::string(buf);
}

std::string getDeviceString(cl_device_id device, cl_device_info param) {
    char buf[256] = {0};
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

// ============================================================
// Main
// ============================================================
int main() {
    std::cout << "==============================================" << std::endl;
    std::cout << "  OpenCL-Benchmark (Standalone)" << std::endl;
    std::cout << "==============================================" << std::endl;
    std::cout << std::endl;

    if (!g_ocl.load()) {
        std::cerr << "Failed to load OpenCL!" << std::endl;
        return 1;
    }

    // Get platforms
    cl_uint num_platforms = 0;
    cl_int err = g_ocl.clGetPlatformIDs(0, nullptr, &num_platforms);
    if (err != CL_SUCCESS) {
        std::cerr << "clGetPlatformIDs failed: " << err << std::endl;
        return 1;
    }
    if (num_platforms == 0) {
        std::cerr << "No OpenCL platforms found!" << std::endl;
        return 1;
    }

    std::cout << "Found " << num_platforms << " platform(s)" << std::endl;
    std::vector<cl_platform_id> platforms(num_platforms);
    g_ocl.clGetPlatformIDs(num_platforms, platforms.data(), nullptr);

    // Find all devices
    std::vector<cl_device_id> all_devices;
    cl_device_id gpu_device = nullptr;
    cl_platform_id gpu_platform = nullptr;

    for (cl_uint p = 0; p < num_platforms; p++) {
        std::string name = getInfoString(platforms[p], CL_PLATFORM_NAME);
        std::string vendor = getInfoString(platforms[p], CL_PLATFORM_VENDOR);
        std::string version = getInfoString(platforms[p], CL_PLATFORM_VERSION);
        std::cout << "\nPlatform " << p << ": " << name << std::endl;
        std::cout << "  Vendor: " << vendor << std::endl;
        std::cout << "  Version: " << version << std::endl;

        for (cl_bitfield dtype : {CL_DEVICE_TYPE_GPU, CL_DEVICE_TYPE_CPU, CL_DEVICE_TYPE_ACCELERATOR}) {
            cl_uint num_devs = 0;
            err = g_ocl.clGetDeviceIDs(platforms[p], dtype, 0, nullptr, &num_devs);
            if (err != CL_SUCCESS || num_devs == 0) continue;

            std::vector<cl_device_id> devs(num_devs);
            g_ocl.clGetDeviceIDs(platforms[p], dtype, num_devs, devs.data(), nullptr);

            for (cl_device_id dev : devs) {
                std::string dname = getDeviceString(dev, CL_DEVICE_NAME);
                std::string dvendor = getDeviceString(dev, CL_DEVICE_VENDOR);
                cl_uint cus = getDeviceUInt(dev, CL_DEVICE_MAX_COMPUTE_UNITS);
                cl_uint freq = getDeviceUInt(dev, CL_DEVICE_MAX_CLOCK_FREQUENCY);
                cl_ulong mem = getDeviceULong(dev, CL_DEVICE_GLOBAL_MEM_SIZE);

                std::cout << "  Device: " << dname << std::endl;
                std::cout << "    Vendor: " << dvendor << std::endl;
                std::cout << "    Compute Units: " << cus << std::endl;
                std::cout << "    Max Frequency: " << freq << " MHz" << std::endl;
                std::cout << "    Global Memory: " << (mem / 1024 / 1024) << " MB" << std::endl;

                all_devices.push_back(dev);
                if (dtype == CL_DEVICE_TYPE_GPU && !gpu_device) {
                    gpu_device = dev;
                    gpu_platform = platforms[p];
                }
            }
        }
    }

    if (all_devices.empty()) {
        std::cerr << "\nNo OpenCL devices found!" << std::endl;
        return 1;
    }

    cl_device_id test_device = gpu_device ? gpu_device : all_devices[0];
    cl_platform_id test_platform = gpu_platform ? gpu_platform : platforms[0];

    std::cout << "\n==============================================" << std::endl;
    std::cout << "  Running Test on: " << getDeviceString(test_device, CL_DEVICE_NAME) << std::endl;
    std::cout << "==============================================" << std::endl;

    // Create context
    cl_int ctx_err = 0;
    cl_context context = g_ocl.clCreateContext(nullptr, 1, &test_device, nullptr, nullptr, &ctx_err);
    if (ctx_err != CL_SUCCESS) {
        std::cerr << "Failed to create context: " << ctx_err << std::endl;
        return 1;
    }

    // Create command queue
    cl_int q_err = 0;
    cl_command_queue queue = g_ocl.clCreateCommandQueue(context, test_device, 0, &q_err);
    if (q_err != CL_SUCCESS) {
        std::cerr << "Failed to create command queue: " << q_err << std::endl;
        g_ocl.clReleaseContext(context);
        return 1;
    }

    // Create and build program
    cl_program program = g_ocl.clCreateProgramWithSource(context, 1, &kernel_code, nullptr, nullptr);
    err = g_ocl.clBuildProgram(program, 1, &test_device, nullptr, nullptr, nullptr);
    if (err != CL_SUCCESS) {
        char log[4096] = {0};
        g_ocl.clGetProgramBuildInfo(program, test_device, 0x1183, sizeof(log), log, nullptr);
        std::cerr << "Build failed: " << err << std::endl;
        std::cerr << "Log: " << log << std::endl;
        g_ocl.clReleaseProgram(program);
        g_ocl.clReleaseCommandQueue(queue);
        g_ocl.clReleaseContext(context);
        return 1;
    }
    std::cout << "Program compiled successfully!" << std::endl;

    // Track test results
    int tests_passed = 0;
    int tests_total = 0;

    // Test 1: Simple vector add
    {
        std::cout << "\n--- Test 1: Vector Addition ---" << std::endl;
        tests_total++;
        const size_t N = 1024;
        std::vector<float> a(N, 1.0f), b(N, 2.0f), c(N, 0.0f);

        cl_mem buf_a = g_ocl.clCreateBuffer(context, CL_MEM_COPY_HOST_PTR | CL_MEM_READ_ONLY,
                                             N * sizeof(float), a.data(), nullptr);
        cl_mem buf_b = g_ocl.clCreateBuffer(context, CL_MEM_COPY_HOST_PTR | CL_MEM_READ_ONLY,
                                             N * sizeof(float), b.data(), nullptr);
        cl_mem buf_c = g_ocl.clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                                             N * sizeof(float), nullptr, nullptr);

        cl_kernel kernel = g_ocl.clCreateKernel(program, "test_add", nullptr);
        g_ocl.clSetKernelArg(kernel, 0, sizeof(cl_mem), &buf_a);
        g_ocl.clSetKernelArg(kernel, 1, sizeof(cl_mem), &buf_b);
        g_ocl.clSetKernelArg(kernel, 2, sizeof(cl_mem), &buf_c);
        int n_val = (int)N;
        g_ocl.clSetKernelArg(kernel, 3, sizeof(int), &n_val);

        size_t global_size = N;
        g_ocl.clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &global_size, nullptr, 0, nullptr, nullptr);
        g_ocl.clFinish(queue);
        g_ocl.clEnqueueReadBuffer(queue, buf_c, CL_TRUE, 0, N * sizeof(float), c.data(), 0, nullptr, nullptr);

        bool correct = true;
        for (size_t i = 0; i < N; i++) {
            if (std::fabs(c[i] - 3.0f) > 0.001f) { correct = false; break; }
        }
        std::cout << "  Result: " << (correct ? "PASSED" : "FAILED") << std::endl;
        if (correct) tests_passed++;

        g_ocl.clReleaseKernel(kernel);
        g_ocl.clReleaseMemObject(buf_a);
        g_ocl.clReleaseMemObject(buf_b);
        g_ocl.clReleaseMemObject(buf_c);
    }

    // Test 2: Compute benchmark
    {
        std::cout << "\n--- Test 2: Compute Benchmark ---" << std::endl;
        tests_total++;
        const size_t N = 1024 * 1024;
        const int iters = 100;

        std::vector<float> data(N, 1.0f);
        cl_mem buf = g_ocl.clCreateBuffer(context, CL_MEM_COPY_HOST_PTR | CL_MEM_READ_WRITE,
                                          N * sizeof(float), data.data(), nullptr);

        cl_kernel kernel = g_ocl.clCreateKernel(program, "benchmark_compute", nullptr);
        g_ocl.clSetKernelArg(kernel, 0, sizeof(cl_mem), &buf);
        int n_val = (int)N;
        g_ocl.clSetKernelArg(kernel, 1, sizeof(int), &n_val);
        g_ocl.clSetKernelArg(kernel, 2, sizeof(int), &iters);

        // Warmup
        size_t global_size = N;
        g_ocl.clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &global_size, nullptr, 0, nullptr, nullptr);
        g_ocl.clFinish(queue);

        // Benchmark
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < 10; i++) {
            g_ocl.clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &global_size, nullptr, 0, nullptr, nullptr);
        }
        g_ocl.clFinish(queue);
        auto end = std::chrono::high_resolution_clock::now();

        double time_ms = std::chrono::duration<double, std::milli>(end - start).count() / 10.0;
        double gflops = (double)N * iters * 2 / (time_ms * 1e6);
        std::cout << "  Time: " << std::fixed << std::setprecision(3) << time_ms << " ms" << std::endl;
        std::cout << "  Performance: " << std::setprecision(2) << gflops << " GFLOPS" << std::endl;
        tests_passed++;

        g_ocl.clReleaseKernel(kernel);
        g_ocl.clReleaseMemObject(buf);
    }

    // Test 3: Memory Bandwidth
    {
        std::cout << "\n--- Test 3: Memory Bandwidth ---" << std::endl;
        tests_total++;
        const size_t N = 4 * 1024 * 1024; // 4M elements = 16MB

        std::vector<float> src(N, 1.0f), dst(N, 0.0f);
        cl_mem buf_src = g_ocl.clCreateBuffer(context, CL_MEM_COPY_HOST_PTR | CL_MEM_READ_ONLY,
                                              N * sizeof(float), src.data(), nullptr);
        cl_mem buf_dst = g_ocl.clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                                              N * sizeof(float), nullptr, nullptr);

        cl_kernel kernel = g_ocl.clCreateKernel(program, "mem_bandwidth", nullptr);
        g_ocl.clSetKernelArg(kernel, 0, sizeof(cl_mem), &buf_src);
        g_ocl.clSetKernelArg(kernel, 1, sizeof(cl_mem), &buf_dst);
        int n_val = (int)N;
        g_ocl.clSetKernelArg(kernel, 2, sizeof(int), &n_val);

        // Warmup
        size_t global_size = N;
        g_ocl.clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &global_size, nullptr, 0, nullptr, nullptr);
        g_ocl.clFinish(queue);

        // Benchmark
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < 10; i++) {
            g_ocl.clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &global_size, nullptr, 0, nullptr, nullptr);
        }
        g_ocl.clFinish(queue);
        auto end = std::chrono::high_resolution_clock::now();

        double time_ms = std::chrono::duration<double, std::milli>(end - start).count() / 10.0;
        double bandwidth = (double)N * sizeof(float) * 2 / (time_ms * 1e6); // Read + Write
        std::cout << "  Time: " << std::fixed << std::setprecision(3) << time_ms << " ms" << std::endl;
        std::cout << "  Bandwidth: " << std::setprecision(2) << bandwidth << " GB/s" << std::endl;
        tests_passed++;

        g_ocl.clReleaseKernel(kernel);
        g_ocl.clReleaseMemObject(buf_src);
        g_ocl.clReleaseMemObject(buf_dst);
    }

    // Test 4: Matrix Multiplication
    {
        std::cout << "\n--- Test 4: Matrix Multiplication (64x64) ---" << std::endl;
        tests_total++;
        const int N = 64;
        std::vector<float> A(N * N, 1.0f), B(N * N, 2.0f), C(N * N, 0.0f);

        cl_mem buf_A = g_ocl.clCreateBuffer(context, CL_MEM_COPY_HOST_PTR | CL_MEM_READ_ONLY,
                                            N * N * sizeof(float), A.data(), nullptr);
        cl_mem buf_B = g_ocl.clCreateBuffer(context, CL_MEM_COPY_HOST_PTR | CL_MEM_READ_ONLY,
                                            N * N * sizeof(float), B.data(), nullptr);
        cl_mem buf_C = g_ocl.clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                                            N * N * sizeof(float), nullptr, nullptr);

        cl_kernel kernel = g_ocl.clCreateKernel(program, "matrix_mul", nullptr);
        g_ocl.clSetKernelArg(kernel, 0, sizeof(cl_mem), &buf_A);
        g_ocl.clSetKernelArg(kernel, 1, sizeof(cl_mem), &buf_B);
        g_ocl.clSetKernelArg(kernel, 2, sizeof(cl_mem), &buf_C);
        g_ocl.clSetKernelArg(kernel, 3, sizeof(int), &N);

        size_t global[2] = {(size_t)N, (size_t)N};
        auto start = std::chrono::high_resolution_clock::now();
        g_ocl.clEnqueueNDRangeKernel(queue, kernel, 2, nullptr, global, nullptr, 0, nullptr, nullptr);
        g_ocl.clFinish(queue);
        auto end = std::chrono::high_resolution_clock::now();

        g_ocl.clEnqueueReadBuffer(queue, buf_C, CL_TRUE, 0, N * N * sizeof(float), C.data(), 0, nullptr, nullptr);

        double time_ms = std::chrono::duration<double, std::milli>(end - start).count();
        // Expected: C[i][j] = sum(A[i][k] * B[k][j]) = N * 1.0 * 2.0 = 2N = 128
        bool correct = std::fabs(C[0] - 128.0f) < 0.001f;
        std::cout << "  Time: " << std::fixed << std::setprecision(3) << time_ms << " ms" << std::endl;
        std::cout << "  Result: " << (correct ? "PASSED" : "FAILED") << " (C[0]=" << C[0] << ", expected=128)" << std::endl;
        if (correct) tests_passed++;

        g_ocl.clReleaseKernel(kernel);
        g_ocl.clReleaseMemObject(buf_A);
        g_ocl.clReleaseMemObject(buf_B);
        g_ocl.clReleaseMemObject(buf_C);
    }

    // Test 5: Local Memory
    {
        std::cout << "\n--- Test 5: Local Memory Test ---" << std::endl;
        tests_total++;
        const size_t N = 256 * 4; // 4 workgroups
        std::vector<float> input(N, 2.0f), output(N, 0.0f);

        cl_mem buf_in = g_ocl.clCreateBuffer(context, CL_MEM_COPY_HOST_PTR | CL_MEM_READ_ONLY,
                                             N * sizeof(float), input.data(), nullptr);
        cl_mem buf_out = g_ocl.clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                                              N * sizeof(float), nullptr, nullptr);

        cl_kernel kernel = g_ocl.clCreateKernel(program, "local_mem_test", nullptr);
        g_ocl.clSetKernelArg(kernel, 0, sizeof(cl_mem), &buf_in);
        g_ocl.clSetKernelArg(kernel, 1, sizeof(cl_mem), &buf_out);
        int n_val = (int)N;
        g_ocl.clSetKernelArg(kernel, 2, sizeof(int), &n_val);

        size_t global_size = N;
        size_t local_size = 256;
        g_ocl.clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &global_size, &local_size, 0, nullptr, nullptr);
        g_ocl.clFinish(queue);
        g_ocl.clEnqueueReadBuffer(queue, buf_out, CL_TRUE, 0, N * sizeof(float), output.data(), 0, nullptr, nullptr);

        // Each output should be average of 256 elements = 2.0
        bool correct = std::fabs(output[0] - 2.0f) < 0.001f;
        std::cout << "  Result: " << (correct ? "PASSED" : "FAILED") << " (output[0]=" << output[0] << ", expected=2.0)" << std::endl;
        if (correct) tests_passed++;

        g_ocl.clReleaseKernel(kernel);
        g_ocl.clReleaseMemObject(buf_in);
        g_ocl.clReleaseMemObject(buf_out);
    }

    // Test 6: Integer Operations
    {
        std::cout << "\n--- Test 6: Integer Operations ---" << std::endl;
        tests_total++;
        const size_t N = 1024;
        std::vector<int> a(N, 10), b(N, 3), c(N, 0);

        cl_mem buf_a = g_ocl.clCreateBuffer(context, CL_MEM_COPY_HOST_PTR | CL_MEM_READ_ONLY,
                                            N * sizeof(int), a.data(), nullptr);
        cl_mem buf_b = g_ocl.clCreateBuffer(context, CL_MEM_COPY_HOST_PTR | CL_MEM_READ_ONLY,
                                            N * sizeof(int), b.data(), nullptr);
        cl_mem buf_c = g_ocl.clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                                            N * sizeof(int), nullptr, nullptr);

        cl_kernel kernel = g_ocl.clCreateKernel(program, "int_operations", nullptr);
        g_ocl.clSetKernelArg(kernel, 0, sizeof(cl_mem), &buf_a);
        g_ocl.clSetKernelArg(kernel, 1, sizeof(cl_mem), &buf_b);
        g_ocl.clSetKernelArg(kernel, 2, sizeof(cl_mem), &buf_c);
        int n_val = (int)N;
        g_ocl.clSetKernelArg(kernel, 3, sizeof(int), &n_val);

        size_t global_size = N;
        g_ocl.clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &global_size, nullptr, 0, nullptr, nullptr);
        g_ocl.clFinish(queue);
        g_ocl.clEnqueueReadBuffer(queue, buf_c, CL_TRUE, 0, N * sizeof(int), c.data(), 0, nullptr, nullptr);

        // Expected: c = a * b + a - b = 10 * 3 + 10 - 3 = 37
        bool correct = c[0] == 37;
        std::cout << "  Result: " << (correct ? "PASSED" : "FAILED") << " (c[0]=" << c[0] << ", expected=37)" << std::endl;
        if (correct) tests_passed++;

        g_ocl.clReleaseKernel(kernel);
        g_ocl.clReleaseMemObject(buf_a);
        g_ocl.clReleaseMemObject(buf_b);
        g_ocl.clReleaseMemObject(buf_c);
    }

    // Test 7: Mixed Precision (Math Functions)
    {
        std::cout << "\n--- Test 7: Math Functions (sin/cos/sqrt) ---" << std::endl;
        tests_total++;
        const size_t N = 1024;
        std::vector<float> a(N, 1.0f), b(N, 0.0f), c(N, 0.0f);

        cl_mem buf_a = g_ocl.clCreateBuffer(context, CL_MEM_COPY_HOST_PTR | CL_MEM_READ_ONLY,
                                            N * sizeof(float), a.data(), nullptr);
        cl_mem buf_b = g_ocl.clCreateBuffer(context, CL_MEM_COPY_HOST_PTR | CL_MEM_READ_ONLY,
                                            N * sizeof(float), b.data(), nullptr);
        cl_mem buf_c = g_ocl.clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                                            N * sizeof(float), nullptr, nullptr);

        cl_kernel kernel = g_ocl.clCreateKernel(program, "mixed_ops", nullptr);
        g_ocl.clSetKernelArg(kernel, 0, sizeof(cl_mem), &buf_a);
        g_ocl.clSetKernelArg(kernel, 1, sizeof(cl_mem), &buf_b);
        g_ocl.clSetKernelArg(kernel, 2, sizeof(cl_mem), &buf_c);
        int n_val = (int)N;
        g_ocl.clSetKernelArg(kernel, 3, sizeof(int), &n_val);

        size_t global_size = N;
        g_ocl.clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &global_size, nullptr, 0, nullptr, nullptr);
        g_ocl.clFinish(queue);
        g_ocl.clEnqueueReadBuffer(queue, buf_c, CL_TRUE, 0, N * sizeof(float), c.data(), 0, nullptr, nullptr);

        // v = 1.0, v = 1.0*1.0 + 0 = 1.0, v = sqrt(1.0) = 1.0, v = sin(1.0) + cos(1.0) ≈ 1.38
        float expected = std::sin(1.0f) + std::cos(1.0f);
        bool correct = std::fabs(c[0] - expected) < 0.01f;
        std::cout << "  Result: " << (correct ? "PASSED" : "FAILED") << " (c[0]=" << c[0] << ", expected=" << expected << ")" << std::endl;
        if (correct) tests_passed++;

        g_ocl.clReleaseKernel(kernel);
        g_ocl.clReleaseMemObject(buf_a);
        g_ocl.clReleaseMemObject(buf_b);
        g_ocl.clReleaseMemObject(buf_c);
    }

    // Test 8: Atomic Operations
    {
        std::cout << "\n--- Test 8: Atomic Operations ---" << std::endl;
        tests_total++;
        const size_t N = 10000;
        int counter_init = 0;
        std::vector<int> counter(1, 0);

        cl_mem buf_counter = g_ocl.clCreateBuffer(context, CL_MEM_COPY_HOST_PTR | CL_MEM_READ_WRITE,
                                                  sizeof(int), counter.data(), nullptr);

        cl_kernel kernel = g_ocl.clCreateKernel(program, "atomic_test", nullptr);
        g_ocl.clSetKernelArg(kernel, 0, sizeof(cl_mem), &buf_counter);
        int n_val = (int)N;
        g_ocl.clSetKernelArg(kernel, 1, sizeof(int), &n_val);

        size_t global_size = N;
        g_ocl.clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &global_size, nullptr, 0, nullptr, nullptr);
        g_ocl.clFinish(queue);
        g_ocl.clEnqueueReadBuffer(queue, buf_counter, CL_TRUE, 0, sizeof(int), counter.data(), 0, nullptr, nullptr);

        bool correct = counter[0] == (int)N;
        std::cout << "  Result: " << (correct ? "PASSED" : "FAILED") << " (counter=" << counter[0] << ", expected=" << N << ")" << std::endl;
        if (correct) tests_passed++;

        g_ocl.clReleaseKernel(kernel);
        g_ocl.clReleaseMemObject(buf_counter);
    }

    // Test 9: Parallel Reduction
    {
        std::cout << "\n--- Test 9: Parallel Reduction ---" << std::endl;
        tests_total++;
        const size_t N = 1024 * 1024;
        const size_t local_size = 256;
        const size_t num_groups = N / local_size;

        std::vector<float> input(N, 1.0f);
        std::vector<float> partial_sums(num_groups, 0.0f);

        cl_mem buf_in = g_ocl.clCreateBuffer(context, CL_MEM_COPY_HOST_PTR | CL_MEM_READ_ONLY,
                                             N * sizeof(float), input.data(), nullptr);
        cl_mem buf_partial = g_ocl.clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                                                  num_groups * sizeof(float), nullptr, nullptr);

        cl_kernel kernel = g_ocl.clCreateKernel(program, "parallel_reduction", nullptr);
        g_ocl.clSetKernelArg(kernel, 0, sizeof(cl_mem), &buf_in);
        g_ocl.clSetKernelArg(kernel, 1, sizeof(cl_mem), &buf_partial);
        int n_val = (int)N;
        g_ocl.clSetKernelArg(kernel, 2, sizeof(int), &n_val);
        size_t local_mem_size = local_size * sizeof(float);
        g_ocl.clSetKernelArg(kernel, 3, local_mem_size, nullptr);

        size_t global_size = N;
        g_ocl.clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &global_size, &local_size, 0, nullptr, nullptr);
        g_ocl.clFinish(queue);
        g_ocl.clEnqueueReadBuffer(queue, buf_partial, CL_TRUE, 0, num_groups * sizeof(float), partial_sums.data(), 0, nullptr, nullptr);

        // Sum all partial results
        float total = 0.0f;
        for (size_t i = 0; i < num_groups; i++) total += partial_sums[i];

        float expected = (float)N;
        bool correct = std::fabs(total - expected) < expected * 0.001f;
        std::cout << "  Result: " << (correct ? "PASSED" : "FAILED") << " (sum=" << (long long)total << ", expected=" << (long long)expected << ")" << std::endl;
        if (correct) tests_passed++;

        g_ocl.clReleaseKernel(kernel);
        g_ocl.clReleaseMemObject(buf_in);
        g_ocl.clReleaseMemObject(buf_partial);
    }

    // Test 10: Vector Types (float4)
    {
        std::cout << "\n--- Test 10: Vector Types (float4) ---" << std::endl;
        tests_total++;
        const size_t N = 1024; // 1024 float4 vectors = 4096 floats
        std::vector<float> a(N * 4, 1.0f), b(N * 4, 2.0f), c(N * 4, 0.0f);

        cl_mem buf_a = g_ocl.clCreateBuffer(context, CL_MEM_COPY_HOST_PTR | CL_MEM_READ_ONLY,
                                            N * 4 * sizeof(float), a.data(), nullptr);
        cl_mem buf_b = g_ocl.clCreateBuffer(context, CL_MEM_COPY_HOST_PTR | CL_MEM_READ_ONLY,
                                            N * 4 * sizeof(float), b.data(), nullptr);
        cl_mem buf_c = g_ocl.clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                                            N * 4 * sizeof(float), nullptr, nullptr);

        cl_kernel kernel = g_ocl.clCreateKernel(program, "vector_types", nullptr);
        g_ocl.clSetKernelArg(kernel, 0, sizeof(cl_mem), &buf_a);
        g_ocl.clSetKernelArg(kernel, 1, sizeof(cl_mem), &buf_b);
        g_ocl.clSetKernelArg(kernel, 2, sizeof(cl_mem), &buf_c);
        int n_val = (int)N;
        g_ocl.clSetKernelArg(kernel, 3, sizeof(int), &n_val);

        size_t global_size = N;
        g_ocl.clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &global_size, nullptr, 0, nullptr, nullptr);
        g_ocl.clFinish(queue);
        g_ocl.clEnqueueReadBuffer(queue, buf_c, CL_TRUE, 0, N * 4 * sizeof(float), c.data(), 0, nullptr, nullptr);

        // Expected: c = a + b = 1.0 + 2.0 = 3.0
        bool correct = std::fabs(c[0] - 3.0f) < 0.001f;
        std::cout << "  Result: " << (correct ? "PASSED" : "FAILED") << " (c[0]=" << c[0] << ", expected=3.0)" << std::endl;
        if (correct) tests_passed++;

        g_ocl.clReleaseKernel(kernel);
        g_ocl.clReleaseMemObject(buf_a);
        g_ocl.clReleaseMemObject(buf_b);
        g_ocl.clReleaseMemObject(buf_c);
    }

    // Cleanup
    g_ocl.clReleaseProgram(program);
    g_ocl.clReleaseCommandQueue(queue);
    g_ocl.clReleaseContext(context);

    std::cout << "\n==============================================" << std::endl;
    std::cout << "  Benchmark completed!" << std::endl;
    std::cout << "  Tests passed: " << tests_passed << "/" << tests_total << std::endl;
    std::cout << "==============================================" << std::endl;

    return 0;
}
