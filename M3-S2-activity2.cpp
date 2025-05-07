#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define N 8  // Total number of elements in the vectors

int main(int argc, char** argv) {
    int rank, size;
    int *v1, *v2, *v3;
    int local_v1[2], local_v2[2], local_v3[2];  // assuming 4 processes, each gets 2 elements
    int total_sum;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int elements_per_proc = N / size;

    if (rank == 0) {
        // Master initializes vectors
        v1 = (int*)malloc(N * sizeof(int));
        v2 = (int*)malloc(N * sizeof(int));
        v3 = (int*)malloc(N * sizeof(int));

        for (int i = 0; i < N; i++) {
            v1[i] = i;
            v2[i] = i * 2;
        }
    }

    // Scatter vectors to all processes
    MPI_Scatter(v1, elements_per_proc, MPI_INT, local_v1, elements_per_proc, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(v2, elements_per_proc, MPI_INT, local_v2, elements_per_proc, MPI_INT, 0, MPI_COMM_WORLD);

    // Add the vectors locally
    for (int i = 0; i < elements_per_proc; i++) {
        local_v3[i] = local_v1[i] + local_v2[i];
    }

    // Gather the results
    MPI_Gather(local_v3, elements_per_proc, MPI_INT, v3, elements_per_proc, MPI_INT, 0, MPI_COMM_WORLD);

    // Reduce to calculate total sum
    int local_sum = 0;
    for (int i = 0; i < elements_per_proc; i++) {
        local_sum += local_v3[i];
    }

    MPI_Reduce(&local_sum, &total_sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Final result vector v3: ");
        for (int i = 0; i < N; i++) {
            printf("%d ", v3[i]);
        }
        printf("\nTotal sum of v3: %d\n", total_sum);

        free(v1);
        free(v2);
        free(v3);
    }

    MPI_Finalize();
    return 0;
}
