// Simple OpenCL Stub Library for cross-compilation
// This provides weak stubs for OpenCL functions, allowing the binary to compile
// The actual OpenCL functions will be loaded at runtime from the system's OpenCL library

#include <cstddef>

extern "C" {

// Define all OpenCL API functions as weak symbols that return error
#define STUB_FUNC(ret, name, ...) \
    __attribute__((weak)) ret name(__VA_ARGS__) { return (ret)-1; }

#define STUB_FUNC_PTR(ret, name, ...) \
    __attribute__((weak)) ret name(__VA_ARGS__) { return (ret)0; }

// Platform API
STUB_FUNC(int, clGetPlatformIDs, unsigned int a, void* b, unsigned int* c)
STUB_FUNC(int, clGetPlatformInfo, void* a, unsigned int b, size_t c, void* d, size_t* e)

// Device API
STUB_FUNC(int, clGetDeviceIDs, void* a, unsigned long b, unsigned int c, void* d, unsigned int* e)
STUB_FUNC(int, clGetDeviceInfo, void* a, unsigned int b, size_t c, void* d, size_t* e)
STUB_FUNC(int, clRetainDevice, void* a)
STUB_FUNC(int, clReleaseDevice, void* a)

// Context API
STUB_FUNC_PTR(void*, clCreateContext, const void* a, unsigned int b, const void* c, void* d, void* e, int* f)
STUB_FUNC_PTR(void*, clCreateContextFromType, const void* a, unsigned long b, void* c, void* d, int* e)
STUB_FUNC(int, clRetainContext, void* a)
STUB_FUNC(int, clReleaseContext, void* a)
STUB_FUNC(int, clGetContextInfo, void* a, unsigned int b, size_t c, void* d, size_t* e)

// Command Queue API
STUB_FUNC_PTR(void*, clCreateCommandQueue, void* a, void* b, unsigned long c, int* d)
STUB_FUNC_PTR(void*, clCreateCommandQueueWithProperties, void* a, void* b, const void* c, int* d)
STUB_FUNC(int, clRetainCommandQueue, void* a)
STUB_FUNC(int, clReleaseCommandQueue, void* a)
STUB_FUNC(int, clGetCommandQueueInfo, void* a, unsigned int b, size_t c, void* d, size_t* e)

// Memory Object API
STUB_FUNC_PTR(void*, clCreateBuffer, void* a, unsigned long b, size_t c, void* d, int* e)
STUB_FUNC(int, clRetainMemObject, void* a)
STUB_FUNC(int, clReleaseMemObject, void* a)
STUB_FUNC(int, clGetMemObjectInfo, void* a, unsigned int b, size_t c, void* d, size_t* e)

// Program API
STUB_FUNC_PTR(void*, clCreateProgramWithSource, void* a, unsigned int b, const char** c, const size_t* d, int* e)
STUB_FUNC_PTR(void*, clCreateProgramWithBinary, void* a, unsigned int b, const void* c, const size_t* d, const unsigned char** e, int* f, int* g)
STUB_FUNC(int, clRetainProgram, void* a)
STUB_FUNC(int, clReleaseProgram, void* a)
STUB_FUNC(int, clBuildProgram, void* a, unsigned int b, const void* c, const char* d, void* e, void* f)
STUB_FUNC(int, clGetProgramInfo, void* a, unsigned int b, size_t c, void* d, size_t* e)
STUB_FUNC(int, clGetProgramBuildInfo, void* a, void* b, unsigned int c, size_t d, void* e, size_t* f)

// Kernel API
STUB_FUNC_PTR(void*, clCreateKernel, void* a, const char* b, int* c)
STUB_FUNC(int, clCreateKernelsInProgram, void* a, unsigned int b, void* c, unsigned int* d)
STUB_FUNC(int, clRetainKernel, void* a)
STUB_FUNC(int, clReleaseKernel, void* a)
STUB_FUNC(int, clSetKernelArg, void* a, unsigned int b, size_t c, const void* d)
STUB_FUNC(int, clGetKernelInfo, void* a, unsigned int b, size_t c, void* d, size_t* e)
STUB_FUNC(int, clGetKernelWorkGroupInfo, void* a, void* b, unsigned int c, size_t d, void* e, size_t* f)

// Event API
STUB_FUNC(int, clWaitForEvents, unsigned int a, const void* b)
STUB_FUNC(int, clGetEventInfo, void* a, unsigned int b, size_t c, void* d, size_t* e)
STUB_FUNC_PTR(void*, clCreateUserEvent, void* a, int* b)
STUB_FUNC(int, clRetainEvent, void* a)
STUB_FUNC(int, clReleaseEvent, void* a)
STUB_FUNC(int, clSetUserEventStatus, void* a, int b)
STUB_FUNC(int, clSetEventCallback, void* a, int b, void* c, void* d)

// Enqueued Objects API
STUB_FUNC(int, clEnqueueReadBuffer, void* a, void* b, unsigned int c, size_t d, size_t e, void* f, unsigned int g, const void* h, void* i)
STUB_FUNC(int, clEnqueueWriteBuffer, void* a, void* b, unsigned int c, size_t d, size_t e, const void* f, unsigned int g, const void* h, void* i)
STUB_FUNC(int, clEnqueueCopyBuffer, void* a, void* b, void* c, size_t d, size_t e, size_t f, unsigned int g, const void* h, void* i)
STUB_FUNC_PTR(void*, clEnqueueMapBuffer, void* a, void* b, unsigned int c, unsigned long d, size_t e, size_t f, unsigned int g, const void* h, void* i, int* j)
STUB_FUNC(int, clEnqueueUnmapMemObject, void* a, void* b, void* c, unsigned int d, const void* e, void* f)
STUB_FUNC(int, clEnqueueNDRangeKernel, void* a, void* b, unsigned int c, const size_t* d, const size_t* e, const size_t* f, unsigned int g, const void* h, void* i)
STUB_FUNC(int, clEnqueueMarker, void* a, void* b)
STUB_FUNC(int, clEnqueueBarrier, void* a)

// Flush and Finish API
STUB_FUNC(int, clFlush, void* a)
STUB_FUNC(int, clFinish, void* a)

// Sampler API
STUB_FUNC_PTR(void*, clCreateSampler, void* a, unsigned int b, unsigned int c, unsigned int d, int* e)
STUB_FUNC(int, clRetainSampler, void* a)
STUB_FUNC(int, clReleaseSampler, void* a)

}
