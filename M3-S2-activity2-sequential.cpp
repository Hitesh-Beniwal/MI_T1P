#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void randomVector(int *vec, int size) {
    for (int i = 0; i < size; i++) {
        vec[i] = rand() % 100;
    }
}

int main(int argc, char** argv) {
    int rank, size;
    const int total_size = 1000; // Change to larger size for benchmarking
    int *v1 = NULL, *v2 = NULL, *v3 = NULL;
    int *local_v1, *local_v2, *local_v3;
    int chunk_size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (total_size % size != 0) {
        if (rank == 0)
            printf("Error: total_size (%d) is not divisible by number of processes (%d).\n", total_size, size);
        MPI_Finalize();
        return 1;
    }

    chunk_size = total_size / size;

    // Allocate memory for local chunks
    local_v1 = (int*)malloc(sizeof(int) * chunk_size);
    local_v2 = (int*)malloc(sizeof(int) * chunk_size);
    local_v3 = (int*)malloc(sizeof(int) * chunk_size);

    // Only the master allocates full vectors
    if (rank == 0) {
        v1 = (int*)malloc(sizeof(int) * total_size);
        v2 = (int*)malloc(sizeof(int) * total_size);
        v3 = (int*)malloc(sizeof(int) * total_size);

        srand(time(0));
        randomVector(v1, total_size);
        randomVector(v2, total_size);
    }

    // Scatter the data to all processes
    MPI_Scatter(v1, chunk_size, MPI_INT, local_v1, chunk_size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(v2, chunk_size, MPI_INT, local_v2, chunk_size, MPI_INT, 0, MPI_COMM_WORLD);

    // Local addition
    for (int i = 0; i < chunk_size; i++) {
        local_v3[i] = local_v1[i] + local_v2[i];
    }

    // Gather the results back to master
    MPI_Gather(local_v3, chunk_size, MPI_INT, v3, chunk_size, MPI_INT, 0, MPI_COMM_WORLD);

    // Calculate total sum using MPI_Reduce
    int local_sum = 0, global_sum = 0;
    for (int i = 0; i < chunk_size; i++) {
        local_sum += local_v3[i];
    }

    MPI_Reduce(&local_sum, &global_sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Total sum of all elements in v3: %d\n", global_sum);
        free(v1); free(v2); free(v3);
    }

    free(local_v1); free(local_v2); free(local_v3);
    MPI_Finalize();
    return 0;
}
