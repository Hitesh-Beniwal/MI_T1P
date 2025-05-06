#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include <chrono> // For measuring execution time

#define PRINT 1

int SZ = 100000000; // Default vector size

int *v1, *v2, *v_out; // Input and output vectors

// OpenCL memory buffers
cl_mem bufV1, bufV2, bufV_out;

// OpenCL environment variables
cl_device_id device_id;
cl_context context;
cl_program program;
cl_kernel kernel;
cl_command_queue queue;
cl_event event = NULL;

int err; // To track error codes

// Function declarations
cl_device_id create_device();
void setup_openCL_device_context_queue_kernel(char *filename, char *kernelname);
cl_program build_program(cl_context ctx, cl_device_id dev, const char *filename);
void setup_kernel_memory();
void copy_kernel_args();
void free_memory();
void init(int *&A, int size);
void print(int *A, int size);

int main(int argc, char **argv) {
    if (argc > 1) {
        SZ = atoi(argv[1]); // Override default vector size from command line
    }

    // Initialize vectors with random data
    init(v1, SZ);
    init(v2, SZ);
    init(v_out, SZ);

    size_t global[1] = {(size_t)SZ}; // 1D global range

    // Optional: print vectors before computation
    print(v1, SZ);
    print(v2, SZ);

    // Set up OpenCL environment and compile kernel
    setup_openCL_device_context_queue_kernel((char *)"./vector_ops_ocl.cl", (char *)"vector_add_ocl");

    // Allocate device memory and transfer input data
    setup_kernel_memory();

    // Set kernel function arguments
    copy_kernel_args();

    // Start measuring kernel execution time
    auto start = std::chrono::high_resolution_clock::now();

    // Enqueue kernel execution
    clEnqueueNDRangeKernel(queue, kernel, 1, NULL, global, NULL, 0, NULL, &event);
    clWaitForEvents(1, &event);

    // Read result back from GPU to host
    clEnqueueReadBuffer(queue, bufV_out, CL_TRUE, 0, SZ * sizeof(int), &v_out[0], 0, NULL, NULL);

    // Print result
    print(v_out, SZ);

    auto stop = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed_time = stop - start;

    // Print kernel execution time
    printf("Kernel Execution Time: %f ms\n", elapsed_time.count());

    // Release all OpenCL and host memory
    free_memory();
}

// Initialize vector with random values
void init(int *&A, int size) {
    A = (int *)malloc(sizeof(int) * size);
    for (long i = 0; i < size; i++) {
        A[i] = rand() % 100;
    }
}

// Helper to print vector contents
void print(int *A, int size) {
    if (PRINT == 0) return;

    if (PRINT == 1 && size > 15) {
        for (long i = 0; i < 5; i++) printf("%d ", A[i]);
        printf(" ..... ");
        for (long i = size - 5; i < size; i++) printf("%d ", A[i]);
    } else {
        for (long i = 0; i < size; i++) printf("%d ", A[i]);
    }
    printf("\n----------------------------\n");
}

// Release OpenCL and host memory
void free_memory() {
    clReleaseMemObject(bufV1);
    clReleaseMemObject(bufV2);
    clReleaseMemObject(bufV_out);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(queue);
    clReleaseProgram(program);
    clReleaseContext(context);

    free(v1);
    free(v2);
    free(v_out);
}

// Set kernel arguments
void copy_kernel_args() {
    clSetKernelArg(kernel, 0, sizeof(int), (void *)&SZ);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&bufV1);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&bufV2);
    clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *)&bufV_out);

    if (err < 0) {
        perror("Couldn't create a kernel argument");
        printf("error = %d", err);
        exit(1);
    }
}

// Allocate OpenCL device memory and write input vectors to device
void setup_kernel_memory() {
    bufV1 = clCreateBuffer(context, CL_MEM_READ_WRITE, SZ * sizeof(int), NULL, NULL);
    bufV2 = clCreateBuffer(context, CL_MEM_READ_WRITE, SZ * sizeof(int), NULL, NULL);
    bufV_out = clCreateBuffer(context, CL_MEM_READ_WRITE, SZ * sizeof(int), NULL, NULL);

    clEnqueueWriteBuffer(queue, bufV1, CL_TRUE, 0, SZ * sizeof(int), &v1[0], 0, NULL, NULL);
    clEnqueueWriteBuffer(queue, bufV2, CL_TRUE, 0, SZ * sizeof(int), &v2[0], 0, NULL, NULL);
}

// Set up OpenCL device, context, queue, and compile kernel
void setup_openCL_device_context_queue_kernel(char *filename, char *kernelname) {
    device_id = create_device();
    cl_int err;

    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err);
    if (err < 0) {
        perror("Couldn't create a context");
        exit(1);
    }

    program = build_program(context, device_id, filename);

    queue = clCreateCommandQueueWithProperties(context, device_id, 0, &err);
    if (err < 0) {
        perror("Couldn't create a command queue");
        exit(1);
    }

    kernel = clCreateKernel(program, kernelname, &err);
    if (err < 0) {
        perror("Couldn't create a kernel");
        printf("error =%d", err);
        exit(1);
    }
}

// Compile OpenCL C kernel from file
cl_program build_program(cl_context ctx, cl_device_id dev, const char *filename) {
    cl_program program;
    FILE *program_handle;
    char *program_buffer, *program_log;
    size_t program_size, log_size;

    // Load kernel source file
    program_handle = fopen(filename, "r");
    if (program_handle == NULL) {
        perror("Couldn't find the program file");
        exit(1);
    }
    fseek(program_handle, 0, SEEK_END);
    program_size = ftell(program_handle);
    rewind(program_handle);
    program_buffer = (char *)malloc(program_size + 1);
    program_buffer[program_size] = '\0';
    fread(program_buffer, sizeof(char), program_size, program_handle);
    fclose(program_handle);

    // Create program from source
    program = clCreateProgramWithSource(ctx, 1, (const char **)&program_buffer, &program_size, &err);
    if (err < 0) {
        perror("Couldn't create the program");
        exit(1);
    }
    free(program_buffer);

    // Compile (build) the OpenCL program
    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if (err < 0) {
        // Print build log in case of error
        clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        program_log = (char *)malloc(log_size + 1);
        program_log[log_size] = '\0';
        clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, log_size + 1, program_log, NULL);
        printf("%s\n", program_log);
        free(program_log);
        exit(1);
    }

    return program;
}

// Detect GPU (or fallback to CPU) for execution
cl_device_id create_device() {
    cl_platform_id platform;
    cl_device_id dev;
    int err;

    err = clGetPlatformIDs(1, &platform, NULL);
    if (err < 0) {
        perror("Couldn't identify a platform");
        exit(1);
    }

    // Try GPU first
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (err == CL_DEVICE_NOT_FOUND) {
        printf("GPU not found\n");
        // Fallback to CPU
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
    }

    if (err < 0) {
        perror("Couldn't access any devices");
        exit(1);
    }

    return dev;
}
