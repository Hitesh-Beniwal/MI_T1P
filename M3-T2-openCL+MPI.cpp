#include <mpi.h> 
#include <CL/cl.h> 
#include <iostream> 
#include <vector> 
#include <cstdlib> 
#include <ctime> 
#include <algorithm> // For std::sort 
 
// Helper macro for OpenCL errors 
#define 
CHECK_CL(err)                                                                \ 
    if (err != 
CL_SUCCESS)                                                           \ 
    {                                                                         
       \ 
        std::cerr << "OpenCL error " << err << " at line " << __LINE__ << 
std::endl; \ 
        exit(1);                                                               
      \ 
    } 
 
const char *kernelSource = R"CLC( 
__kernel void bubbleSort(__global int* data, const int size) { 
    // A simple (sequential) bubble sort 
    for (int i = 0; i < size; i++) { 
        for (int j = 0; j < size - 1 - i; j++) { 
            if (data[j] > data[j+1]) { 
                int tmp = data[j]; 
                data[j] = data[j+1]; 
                data[j+1] = tmp; 
            } 
        } 
    } 
} 
)CLC"; 
 
void opencl_sort(int *data, int size) 
{ 
    // 1) Check for any OpenCL platform 
    cl_uint numPlatforms = 0; 
    cl_int err = clGetPlatformIDs(0, nullptr, &numPlatforms); 
    if (err != CL_SUCCESS || numPlatforms == 0) 
    { 
        std::cerr << "[Rank] No OpenCL platform found; falling back to 
std::sort\n"; 
        std::sort(data, data + size); 
        return; 
    } 
 
    // 2) Pick the first platform 
    cl_platform_id platform; 
    err = clGetPlatformIDs(1, &platform, nullptr); 
    CHECK_CL(err); 
 
    // 3) Try to get a GPU device 
    cl_device_id device = nullptr; 
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, nullptr); 
    if (err == CL_DEVICE_NOT_FOUND) 
    { 
        std::cerr << "[Rank] No GPU device found on platform; falling back to 
std::sort\n"; 
        std::sort(data, data + size); 
        return; 
    } 
    CHECK_CL(err); 
 
    // 4) Create context and command queue 
    cl_context context = clCreateContext(nullptr, 1, &device, nullptr, 
nullptr, &err); 
    CHECK_CL(err); 
    cl_command_queue queue = clCreateCommandQueue(context, device, 0, &err); 
    CHECK_CL(err); 
 
    // 5) Build the program and create kernel 
    cl_program program = clCreateProgramWithSource(context, 1, &kernelSource, 
nullptr, &err); 
    CHECK_CL(err); 
    err = clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr); 
    CHECK_CL(err); 
    cl_kernel kernel = clCreateKernel(program, "bubbleSort", &err); 
    CHECK_CL(err); 
 
    // 6) Create buffer and set kernel args 
    cl_mem buffer = clCreateBuffer(context, 
                                   CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
                                   sizeof(int) * size, 
                                   data, 
                                   &err); 
    CHECK_CL(err); 
    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer); 
    CHECK_CL(err); 
    err = clSetKernelArg(kernel, 1, sizeof(int), &size); 
    CHECK_CL(err); 
 
    // 7) Enqueue kernel with a single work-item 
    size_t global_work_size = 1; 
    err = clEnqueueNDRangeKernel(queue, 
                                 kernel, 
                                 1, 
                                 nullptr, 
                                 &global_work_size, 
                                 nullptr, 
                                 0, 
                                 nullptr, 
                                 nullptr); 
    CHECK_CL(err); 
    clFinish(queue); 
 
    // 8) Read back results 
    err = clEnqueueReadBuffer(queue, 
                              buffer, 
                              CL_TRUE, 
                              0, 
                              sizeof(int) * size, 
                              data, 
                              0, 
                              nullptr, 
                              nullptr); 
    CHECK_CL(err); 
 
    // 9) Cleanup 
    clReleaseMemObject(buffer); 
    clReleaseKernel(kernel); 
    clReleaseProgram(program); 
    clReleaseCommandQueue(queue); 
    clReleaseContext(context); 
} 
 
int main(int argc, char *argv[]) 
{ 
    MPI_Init(&argc, &argv); 
    int rank, world_size; 
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); 
    MPI_Comm_size(MPI_COMM_WORLD, &world_size); 
 
    const int SIZE = 500000; 
    int *full_array = nullptr; 
    int chunk_size = SIZE / world_size; 
    int *local_data = new int[chunk_size]; 
 
    if (rank == 0) 
    { 
        full_array = new int[SIZE]; 
        srand((unsigned)time(nullptr)); 
        for (int i = 0; i < SIZE; i++) 
            full_array[i] = rand() % 1000000; 
    } 
 
    // Distribute chunks 
    MPI_Scatter(full_array, chunk_size, MPI_INT, 
                local_data, chunk_size, MPI_INT, 
                0, MPI_COMM_WORLD); 
 
    // Sort locally (either via GPU bubbleSort or std::sort) 
    double t0 = MPI_Wtime(); 
    opencl_sort(local_data, chunk_size); 
    double t1 = MPI_Wtime(); 
// Gather sorted chunks back 
MPI_Gather(local_data, chunk_size, MPI_INT, 
full_array, chunk_size, MPI_INT, 
0, MPI_COMM_WORLD); 
if (rank == 0) 
{ 
std::cout << "MPI + OpenCL Sort (per-chunk) took " 
<< (t1 - t0) 
<< " seconds\n"; 
delete[] full_array; 
} 
delete[] local_data; 
MPI_Finalize(); 
return 0; 
} 
