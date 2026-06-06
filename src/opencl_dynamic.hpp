// Standalone OpenCL Dynamic Loader
// This file provides a minimal OpenCL implementation that can be
// linked without requiring system OpenCL headers
// It uses dlopen to load OpenCL at runtime

#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <vector>
#include <string>
#include <iostream>
#include <chrono>
#include <thread>
#include <dlfcn.h>

// OpenCL types (standard)
typedef int8_t  cl_int8;
typedef int16_t cl_int16;
typedef int32_t cl_int32;
typedef int64_t cl_int64;
typedef uint8_t  cl_uint8;
typedef uint16_t cl_uint16;
typedef uint32_t cl_uint32;
typedef uint64_t cl_uint64;
typedef uint16_t cl_half;
typedef float cl_float;
typedef double cl_double;
typedef int64_t cl_long;
typedef uint64_t cl_ulong;
typedef int32_t cl_bool;
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
typedef cl_bitfield cl_sampler_info;
typedef cl_bitfield cl_command_queue_properties;
typedef cl_bitfield cl_mem_flags;
typedef cl_bitfield cl_map_flags;
typedef cl_bitfield cl_addressing_mode;
typedef cl_bitfield cl_filter_mode;
typedef void* cl_context_properties;
typedef void* cl_program_properties;

// Opaque types
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
#define CL_SUCCESS                                  0
#define CL_DEVICE_NOT_FOUND                         -1
#define CL_DEVICE_NOT_AVAILABLE                     -2
#define CL_COMPILER_NOT_AVAILABLE                   -3
#define CL_MEM_OBJECT_ALLOCATION_FAILURE            -4
#define CL_OUT_OF_RESOURCES                         -5
#define CL_OUT_OF_HOST_MEMORY                       -6
#define CL_PROFILING_INFO_NOT_AVAILABLE             -7
#define CL_MEM_COPY_OVERLAP                         -8
#define CL_PROFILING_RESULT_NOT_AVAILABLE           -9
#define CL_MEM_ALLOC_REFERENCE_FAILURE              -10
#define CL_BUILD_PROGRAM_FAILURE                    -11
#define CL_MAP_FAILURE                              -12
#define CL_MISALIGNED_SUB_BUFFER_OFFSET             -13
#define CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST -14
#define CL_COMPILE_PROGRAM_FAILURE                  -15
#define CL_LINKER_NOT_AVAILABLE                     -16
#define CL_LINK_PROGRAM_FAILURE                     -17
#define CL_DEVICE_PARTITION_FAILED                  -18
#define CL_KERNEL_ARG_INFO_NOT_AVAILABLE            -19
#define CL_INVALID_VALUE                            -30
#define CL_INVALID_DEVICE_TYPE                      -31
#define CL_INVALID_PLATFORM                         -32
#define CL_INVALID_DEVICE                           -33
#define CL_INVALID_CONTEXT                          -34
#define CL_INVALID_QUEUE_PROPERTIES                 -35
#define CL_INVALID_COMMAND_QUEUE                    -36
#define CL_INVALID_HOST_PTR                         -37
#define CL_INVALID_MEM_OBJECT                       -38
#define CL_INVALID_IMAGE_FORMAT_DESCRIPTOR          -39
#define CL_INVALID_IMAGE_SIZE                       -40
#define CL_INVALID_SAMPLER                          -41
#define CL_INVALID_BINARY                           -42
#define CL_INVALID_BUILD_OPTIONS                    -43
#define CL_INVALID_PROGRAM                          -44
#define CL_INVALID_PROGRAM_EXECUTABLE               -45
#define CL_INVALID_KERNEL_NAME                      -46
#define CL_INVALID_KERNEL_DEFINITION                -47
#define CL_INVALID_KERNEL                           -48
#define CL_INVALID_ARG_INDEX                        -49
#define CL_INVALID_ARG_VALUE                        -50
#define CL_INVALID_ARG_SIZE                         -51
#define CL_INVALID_KERNEL_ARGS                      -52
#define CL_INVALID_WORK_DIMENSION                   -53
#define CL_INVALID_WORK_GROUP_SIZE                  -54
#define CL_INVALID_WORK_ITEM_SIZE                   -55
#define CL_INVALID_GLOBAL_OFFSET                    -56
#define CL_INVALID_EVENT_WAIT_LIST                  -57
#define CL_INVALID_EVENT                            -58
#define CL_INVALID_OPERATION                        -59
#define CL_INVALID_GL_OBJECT                        -60
#define CL_INVALID_BUFFER_SIZE                      -61
#define CL_INVALID_MIP_LEVEL                        -62
#define CL_INVALID_GLOBAL_WORK_SIZE                 -63
#define CL_INVALID_PROPERTY                         -64
#define CL_INVALID_IMAGE_DESCRIPTOR                 -65
#define CL_INVALID_COMPILER_OPTIONS                 -66
#define CL_INVALID_LINKER_OPTIONS                   -67
#define CL_INVALID_DEVICE_PARTITION_COUNT           -68

// Device types
#define CL_DEVICE_TYPE_DEFAULT                      (1 << 0)
#define CL_DEVICE_TYPE_CPU                          (1 << 1)
#define CL_DEVICE_TYPE_GPU                          (1 << 2)
#define CL_DEVICE_TYPE_ACCELERATOR                  (1 << 3)
#define CL_DEVICE_TYPE_CUSTOM                       (1 << 4)
#define CL_DEVICE_TYPE_ALL                          0xFFFFFFFF

// Function pointer types
typedef cl_int (*clGetPlatformIDs_fn)(cl_uint, cl_platform_id*, cl_uint*);
typedef cl_int (*clGetPlatformInfo_fn)(cl_platform_id, cl_platform_info, size_t, void*, size_t*);
typedef cl_int (*clGetDeviceIDs_fn)(cl_platform_id, cl_device_type, cl_uint, cl_device_id*, cl_uint*);
typedef cl_int (*clGetDeviceInfo_fn)(cl_device_id, cl_device_info, size_t, void*, size_t*);
typedef cl_context (*clCreateContext_fn)(const cl_context_properties*, cl_uint, const cl_device_id*, void(*)(const char*, const void*, size_t, void*), void*, cl_int*);
typedef cl_int (*clReleaseContext_fn)(cl_context);
typedef cl_int (*clGetContextInfo_fn)(cl_context, cl_context_info, size_t, void*, size_t*);
typedef cl_command_queue (*clCreateCommandQueue_fn)(cl_context, cl_device_id, cl_command_queue_properties, cl_int*);
typedef cl_int (*clReleaseCommandQueue_fn)(cl_command_queue);
typedef cl_int (*clFinish_fn)(cl_command_queue);
typedef cl_mem (*clCreateBuffer_fn)(cl_context, cl_mem_flags, size_t, void*, cl_int*);
typedef cl_int (*clReleaseMemObject_fn)(cl_mem);
typedef cl_int (*clEnqueueReadBuffer_fn)(cl_command_queue, cl_mem, cl_bool, size_t, size_t, void*, cl_uint, const cl_event*, cl_event*);
typedef cl_int (*clEnqueueWriteBuffer_fn)(cl_command_queue, cl_mem, cl_bool, size_t, size_t, const void*, cl_uint, const cl_event*, cl_event*);
typedef cl_int (*clEnqueueCopyBuffer_fn)(cl_command_queue, cl_mem, cl_mem, size_t, size_t, size_t, cl_uint, const cl_event*, cl_event*);
typedef cl_program (*clCreateProgramWithSource_fn)(cl_context, cl_uint, const char**, const size_t*, cl_int*);
typedef cl_int (*clReleaseProgram_fn)(cl_program);
typedef cl_int (*clBuildProgram_fn)(cl_program, cl_uint, const cl_device_id*, const char*, void(*)(cl_program, void*), void*);
typedef cl_int (*clGetProgramBuildInfo_fn)(cl_program, cl_device_id, cl_build_info, size_t, void*, size_t*);
typedef cl_kernel (*clCreateKernel_fn)(cl_program, const char*, cl_int*);
typedef cl_int (*clReleaseKernel_fn)(cl_kernel);
typedef cl_int (*clSetKernelArg_fn)(cl_kernel, cl_uint, size_t, const void*);
typedef cl_int (*clEnqueueNDRangeKernel_fn)(cl_command_queue, cl_kernel, cl_uint, const size_t*, const size_t*, const size_t*, cl_uint, const cl_event*, cl_event*);

// Dynamic loader
class OpenCLLoader {
public:
    void* lib = nullptr;
    bool loaded = false;

    // Function pointers
    clGetPlatformIDs_fn clGetPlatformIDs = nullptr;
    clGetPlatformInfo_fn clGetPlatformInfo = nullptr;
    clGetDeviceIDs_fn clGetDeviceIDs = nullptr;
    clGetDeviceInfo_fn clGetDeviceInfo = nullptr;
    clCreateContext_fn clCreateContext = nullptr;
    clReleaseContext_fn clReleaseContext = nullptr;
    clGetContextInfo_fn clGetContextInfo = nullptr;
    clCreateCommandQueue_fn clCreateCommandQueue = nullptr;
    clReleaseCommandQueue_fn clReleaseCommandQueue = nullptr;
    clFinish_fn clFinish = nullptr;
    clCreateBuffer_fn clCreateBuffer = nullptr;
    clReleaseMemObject_fn clReleaseMemObject = nullptr;
    clEnqueueReadBuffer_fn clEnqueueReadBuffer = nullptr;
    clEnqueueWriteBuffer_fn clEnqueueWriteBuffer = nullptr;
    clEnqueueCopyBuffer_fn clEnqueueCopyBuffer = nullptr;
    clCreateProgramWithSource_fn clCreateProgramWithSource = nullptr;
    clReleaseProgram_fn clReleaseProgram = nullptr;
    clBuildProgram_fn clBuildProgram = nullptr;
    clGetProgramBuildInfo_fn clGetProgramBuildInfo = nullptr;
    clCreateKernel_fn clCreateKernel = nullptr;
    clReleaseKernel_fn clReleaseKernel = nullptr;
    clSetKernelArg_fn clSetKernelArg = nullptr;
    clEnqueueNDRangeKernel_fn clEnqueueNDRangeKernel = nullptr;

    bool load() {
        // Try multiple library names
        const char* lib_names[] = {
            "libOpenCL.so",
            "libOpenCL.so.1",
            "libmali.so",
            "libMali.so",
            "libGLES_mali.so",  // Mali GPU on Android
            "libGLESv2_mali.so",
            "libOpenCL.so.1.1",
            "libOpenCL.so.1.2",
            "libOpenCL.so.2.0",
            "libOpenCL.so.3.0",
            "libPOCL.so",
            "libpocl.so",
            "libCL.so",
            "libcl.so"
        };

        for (const char* name : lib_names) {
            lib = dlopen(name, RTLD_NOW | RTLD_GLOBAL);
            if (lib) {
                std::cout << "Loaded OpenCL library: " << name << std::endl;
                break;
            }
        }

        if (!lib) {
            std::cerr << "Failed to load OpenCL library: " << dlerror() << std::endl;
            std::cerr << "Searched libraries: ";
            for (const char* name : lib_names) std::cerr << name << " ";
            std::cerr << std::endl;
            return false;
        }

        // Load all functions
        #define LOAD_FUNC(name) \
            name = (name##_fn)dlsym(lib, #name); \
            if (!name) { std::cerr << "Failed to load " << #name << std::endl; }

        LOAD_FUNC(clGetPlatformIDs)
        LOAD_FUNC(clGetPlatformInfo)
        LOAD_FUNC(clGetDeviceIDs)
        LOAD_FUNC(clGetDeviceInfo)
        LOAD_FUNC(clCreateContext)
        LOAD_FUNC(clReleaseContext)
        LOAD_FUNC(clGetContextInfo)
        LOAD_FUNC(clCreateCommandQueue)
        LOAD_FUNC(clReleaseCommandQueue)
        LOAD_FUNC(clFinish)
        LOAD_FUNC(clCreateBuffer)
        LOAD_FUNC(clReleaseMemObject)
        LOAD_FUNC(clEnqueueReadBuffer)
        LOAD_FUNC(clEnqueueWriteBuffer)
        LOAD_FUNC(clEnqueueCopyBuffer)
        LOAD_FUNC(clCreateProgramWithSource)
        LOAD_FUNC(clReleaseProgram)
        LOAD_FUNC(clBuildProgram)
        LOAD_FUNC(clGetProgramBuildInfo)
        LOAD_FUNC(clCreateKernel)
        LOAD_FUNC(clReleaseKernel)
        LOAD_FUNC(clSetKernelArg)
        LOAD_FUNC(clEnqueueNDRangeKernel)

        #undef LOAD_FUNC

        loaded = true;
        return true;
    }

    ~OpenCLLoader() {
        if (lib) dlclose(lib);
    }
};

// Global loader
static OpenCLLoader g_ocl;
