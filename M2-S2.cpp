#include <chrono>
#include <cstdlib>
#include <iostream>
#include <pthread.h>
#include <time.h>

using namespace std;
using namespace std::chrono;

#define NUM_THREADS 4  // Define the number of threads

// Structure to pass arguments to threads
struct ThreadData {
    int *v1, *v2, *v3;
    unsigned long start, end;
};

// Function to initialize a vector with random values
void randomVector(int vector[], unsigned long size) {
    for (unsigned long i = 0; i < size; i++) {
        vector[i] = rand() % 100;  // Assign random values from 0 to 99
    }
}

// Function executed by each thread for partial vector addition
void *parallelAddition(void *args) {
    ThreadData *data = (ThreadData *)args;
    for (unsigned long i = data->start; i < data->end; i++) {
        data->v3[i] = data->v1[i] + data->v2[i];
    }
    pthread_exit(NULL);
}

int main() {
    unsigned long size = 100000000; // Size of vectors (100 million elements)
    srand(time(0));

    int *v1, *v2, *v3;
    
    // Allocate memory for the vectors
    v1 = (int *)malloc(size * sizeof(int));
    v2 = (int *)malloc(size * sizeof(int));
    v3 = (int *)malloc(size * sizeof(int));

    // Initialize vectors with random values
    randomVector(v1, size);
    randomVector(v2, size);

    //  Sequential Execution
    auto start_seq = high_resolution_clock::now();
    for (unsigned long i = 0; i < size; i++) {
        v3[i] = v1[i] + v2[i];
    }
    auto stop_seq = high_resolution_clock::now();
    auto duration_seq = duration_cast<microseconds>(stop_seq - start_seq);
    cout << "Sequential Execution Time: " << duration_seq.count() << " microseconds" << endl;

    //  Parallel Execution
    auto start_par = high_resolution_clock::now();
    
    pthread_t threads[NUM_THREADS]; // Thread pool
    ThreadData threadData[NUM_THREADS]; // Thread argument data
    unsigned long chunk_size = size / NUM_THREADS; // Partition size

    // Create threads and assign work
    for (int i = 0; i < NUM_THREADS; i++) {
        threadData[i].v1 = v1;
        threadData[i].v2 = v2;
        threadData[i].v3 = v3;
        threadData[i].start = i * chunk_size;
        threadData[i].end = (i == NUM_THREADS - 1) ? size : (i + 1) * chunk_size;
        pthread_create(&threads[i], NULL, parallelAddition, (void *)&threadData[i]);
    }

    // Wait for all threads to finish execution
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    auto stop_par = high_resolution_clock::now();
    auto duration_par = duration_cast<microseconds>(stop_par - start_par);
    cout << "Parallel Execution Time: " << duration_par.count() << " microseconds" << endl;

    // Speedup Calculation
    double speedup = (double)duration_seq.count() / duration_par.count();
    cout << "Speedup Achieved: " << speedup << "x" << endl;

    // Free allocated memory
    free(v1);
    free(v2);
    free(v3);

    return 0;
}
