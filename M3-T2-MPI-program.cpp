#include <mpi.h> 
#include <iostream> 
#include <algorithm> 
#include <cstdlib> 
#include <ctime> 
 
using namespace std; 
 
// Sequential quicksort function 
void quickSort(int arr[], int low, int high) { 
    if (low < high) { 
        int pivot = arr[high]; 
        int i = low - 1; 
        for (int j = low; j < high; j++) { 
            if (arr[j] < pivot) { 
                i++; 
                swap(arr[i], arr[j]); 
            } 
        } 
        swap(arr[i + 1], arr[high]); 
        int pi = i + 1; 
        quickSort(arr, low, pi - 1); 
        quickSort(arr, pi + 1, high); 
    } 
} 
 
int main(int argc, char* argv[]) { 
    MPI_Init(&argc, &argv); 
 
    int rank, size; 
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); 
    MPI_Comm_size(MPI_COMM_WORLD, &size); 
 
    const int SIZE = 500000; 
    int* arr = nullptr; 
    int* local_arr; 
    int chunk_size = SIZE / size; 
 
    if (rank == 0) { 
        arr = new int[SIZE]; 
        srand(time(0)); 
        for (int i = 0; i < SIZE; i++) 
            arr[i] = rand() % 1000000; 
    } 
 
    local_arr = new int[chunk_size]; 
 
    double start_time = MPI_Wtime(); 
    MPI_Scatter(arr, chunk_size, MPI_INT, local_arr, chunk_size, MPI_INT, 0, 
MPI_COMM_WORLD); 
 
    quickSort(local_arr, 0, chunk_size - 1); 
 
    MPI_Gather(local_arr, chunk_size, MPI_INT, arr, chunk_size, MPI_INT, 0, 
MPI_COMM_WORLD); 
    double end_time = MPI_Wtime(); 
 
    if (rank == 0) { 
        // Final merge (not parallel, just a quick multi-way merge) 
        inplace_merge(arr, arr + chunk_size, arr + 2 * chunk_size); 
        if (size > 2) 
            inplace_merge(arr, arr + 2 * chunk_size, arr + 3 * chunk_size); 
        if (size > 3) 
            inplace_merge(arr, arr + 3 * chunk_size, arr + 4 * chunk_size); 
 
        cout << "MPI QuickSort Time: " << end_time - start_time << " seconds" 
<< endl; 
        delete[] arr; 
    } 
 
    delete[] local_arr; 
    MPI_Finalize(); 
    return 0; 
} 
