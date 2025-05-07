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
    const int total_size = 100000000;
    int *v1 = NULL, *v2 = NULL, *v3 = NULL;
    int *local_v1, *local_v2, *local_v3;
    int chunk_size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    chunk_size = total_size / size;

    local_v1 = (int*)malloc(sizeof(int) * chunk_size);
    local_v2 = (int*)malloc(sizeof(int) * chunk_size);
    local_v3 = (int*)malloc(sizeof(int) * chunk_size);

    if (rank == 0) {
        v1 = (int*)malloc(sizeof(int) * total_size);
        v2 = (int*)malloc(sizeof(int) * total_size);
        v3 = (int*)malloc(sizeof(int) * total_size);

        srand(time(0));
        randomVector(v1, total_size);
        randomVector(v2, total_size);
    }

    double start = MPI_Wtime();

    MPI_Scatter(v1, chunk_size, MPI_INT, local_v1, chunk_size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(v2, chunk_size, MPI_INT, local_v2, chunk_size, MPI_INT, 0, MPI_COMM_WORLD);

    for (int i = 0; i < chunk_size; i++) {
        local_v3[i] = local_v1[i] + local_v2[i];
    }

    MPI_Gather(local_v3, chunk_size, MPI_INT, v3, chunk_size, MPI_INT, 0, MPI_COMM_WORLD);

    double end = MPI_Wtime();

    if (rank == 0) {
        printf("Vector addition completed in %f seconds.\n", end - start);
        // Free global vectors
        free(v1); free(v2); free(v3);
    }

    free(local_v1); free(local_v2); free(local_v3);
    MPI_Finalize();
    return 0;
}
