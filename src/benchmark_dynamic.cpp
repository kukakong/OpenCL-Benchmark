// OpenCL Benchmark - Dynamic Loading Version
// Works on systems where standard OpenCL library is not available
// Uses dlopen to load OpenCL at runtime

#include "opencl_dynamic.hpp"
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>

// Kernel code as string
static const char* kernel_code =
"__kernel void test_kernel(__global float* a, __global float* b, __global float* c, const int n) {\n"
"    int i = get_global_id(0);\n"
"    if (i < n) {\n"
"        c[i] = a[i] + b[i];\n"
"    }\n"
"}\n";

int main() {
    std::cout << "==============================================" << std::endl;
    std::cout << "  OpenCL-Benchmark (Dynamic Loading Version)" << std::endl;
    std::cout << "==============================================" << std::endl;
    std::cout << std::endl;

    // Load OpenCL
    if (!g_ocl.load()) {
        std::cerr << "Failed to load OpenCL library!" << std::endl;
        std::cerr << std::endl;
        std::cerr << "Please install OpenCL drivers:" << std::endl;
        std::cerr << "  Android: libGLES_mali.so should be in /vendor/lib*/egl/" << std::endl;
        std::cerr << "  Linux:   Install ocl-icd-libopencl1 or vendor-specific drivers" << std::endl;
        return 1;
    }

    // Get platforms
    cl_uint num_platforms = 0;
    cl_int err = g_ocl.clGetPlatformIDs(0, nullptr, &num_platforms);
    if (err != CL_SUCCESS || num_platforms == 0) {
        std::cerr << "No OpenCL platforms found (error: " << err << ")" << std::endl;
        return 1;
    }
    std::cout << "Found " << num_platforms << " OpenCL platform(s)" << std::endl;

    std::vector<cl_platform_id> platforms(num_platforms);
    err = g_ocl.clGetPlatformIDs(num_platforms, platforms.data(), nullptr);
    if (err != CL_SUCCESS) {
        std::cerr << "Failed to get platform IDs (error: " << err << ")" << std::endl;
        return 1;
    }

    // Iterate platforms
    for (cl_uint p = 0; p < num_platforms; p++) {
        char platform_name[256] = {0};
        g_ocl.clGetPlatformInfo(platforms[p], 0x0902, sizeof(platform_name), platform_name, nullptr);
        std::cout << "Platform " << p << ": " << platform_name << std::endl;

        // Get GPU devices
        cl_uint num_devices = 0;
        err = g_ocl.clGetDeviceIDs(platforms[p], CL_DEVICE_TYPE_GPU, 0, nullptr, &num_devices);
        if (err != CL_SUCCESS) {
            std::cout << "  No GPU devices (trying all)..." << std::endl;
            err = g_ocl.clGetDeviceIDs(platforms[p], CL_DEVICE_TYPE_ALL, 0, nullptr, &num_devices);
        }

        if (err != CL_SUCCESS || num_devices == 0) {
            std::cout << "  No devices found" << std::endl;
            continue;
        }

        std::cout << "  Found " << num_devices << " device(s)" << std::endl;

        std::vector<cl_device_id> devices(num_devices);
        g_ocl.clGetDeviceIDs(platforms[p], CL_DEVICE_TYPE_ALL, num_devices, devices.data(), nullptr);

        // Test with first device
        cl_device_id device = devices[0];
        char device_name[256] = {0};
        g_ocl.clGetDeviceInfo(device, 0x102B, sizeof(device_name), device_name, nullptr);
        std::cout << "  Using device: " << device_name << std::endl;

        // Create context
        cl_int ctx_err;
        cl_context context = g_ocl.clCreateContext(nullptr, 1, &device, nullptr, nullptr, &ctx_err);
        if (ctx_err != CL_SUCCESS) {
            std::cerr << "  Failed to create context (error: " << ctx_err << ")" << std::endl;
            continue;
        }

        // Create command queue
        cl_int q_err;
        cl_command_queue queue = g_ocl.clCreateCommandQueue(context, device, 0, &q_err);
        if (q_err != CL_SUCCESS) {
            std::cerr << "  Failed to create command queue (error: " << q_err << ")" << std::endl;
            g_ocl.clReleaseContext(context);
            continue;
        }

        // Create buffers
        const size_t N = 1024;
        std::vector<float> host_a(N, 1.0f);
        std::vector<float> host_b(N, 2.0f);
        std::vector<float> host_c(N, 0.0f);

        cl_mem buf_a = g_ocl.clCreateBuffer(context, 0x03, N * sizeof(float), host_a.data(), nullptr);
        cl_mem buf_b = g_ocl.clCreateBuffer(context, 0x03, N * sizeof(float), host_b.data(), nullptr);
        cl_mem buf_c = g_ocl.clCreateBuffer(context, 0x03, N * sizeof(float), nullptr, nullptr);

        // Create program
        cl_program program = g_ocl.clCreateProgramWithSource(context, 1, &kernel_code, nullptr, nullptr);

        // Build program
        err = g_ocl.clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr);
        if (err != CL_SUCCESS) {
            char build_log[4096] = {0};
            g_ocl.clGetProgramBuildInfo(program, device, 0x1183, sizeof(build_log), build_log, nullptr);
            std::cerr << "  Build failed (error: " << err << ")" << std::endl;
            std::cerr << "  Build log: " << build_log << std::endl;
        } else {
            // Create kernel
            cl_kernel kernel = g_ocl.clCreateKernel(program, "test_kernel", nullptr);

            // Set arguments
            g_ocl.clSetKernelArg(kernel, 0, sizeof(cl_mem), &buf_a);
            g_ocl.clSetKernelArg(kernel, 1, sizeof(cl_mem), &buf_b);
            g_ocl.clSetKernelArg(kernel, 2, sizeof(cl_mem), &buf_c);
            int n_val = (int)N;
            g_ocl.clSetKernelArg(kernel, 3, sizeof(int), &n_val);

            // Execute
            size_t global_size = N;
            g_ocl.clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &global_size, nullptr, 0, nullptr, nullptr);
            g_ocl.clFinish(queue);

            // Read back
            g_ocl.clEnqueueReadBuffer(queue, buf_c, CL_TRUE, 0, N * sizeof(float), host_c.data(), 0, nullptr, nullptr);

            // Verify
            bool correct = true;
            for (size_t i = 0; i < N; i++) {
                if (std::abs(host_c[i] - 3.0f) > 0.001f) {
                    correct = false;
                    break;
                }
            }

            std::cout << "  Test result: " << (correct ? "PASSED" : "FAILED") << std::endl;
            if (correct) {
                std::cout << "  Sample: c[0]=" << host_c[0] << ", c[100]=" << host_c[100] << std::endl;
            }

            g_ocl.clReleaseKernel(kernel);
        }

        // Cleanup
        g_ocl.clReleaseMemObject(buf_a);
        g_ocl.clReleaseMemObject(buf_b);
        g_ocl.clReleaseMemObject(buf_c);
        g_ocl.clReleaseProgram(program);
        g_ocl.clReleaseCommandQueue(queue);
        g_ocl.clReleaseContext(context);
    }

    std::cout << std::endl;
    std::cout << "Benchmark completed successfully!" << std::endl;
    return 0;
}
