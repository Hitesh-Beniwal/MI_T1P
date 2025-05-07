#include <mpi.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv) {
    int rank, size;
    char message[100];

    MPI_Init(&argc, &argv);                      // Initialize MPI environment
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);        // Get current process rank
    MPI_Comm_size(MPI_COMM_WORLD, &size);        // Get number of processes

    if (rank == 0) {
        // Master process
        strcpy(message, "Hello World!");
        for (int i = 1; i < size; i++) {
            MPI_Send(message, strlen(message) + 1, MPI_CHAR, i, 0, MPI_COMM_WORLD);
        }
    } else {
        // Worker processes
        MPI_Recv(message, 100, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Process %d received message: %s\n", rank, message);
    }

    // Now using Broadcast
    if (rank == 0) {
        strcpy(message, "Hello World! (Broadcast)");
    }

    MPI_Bcast(message, 100, MPI_CHAR, 0, MPI_COMM_WORLD);
    printf("Process %d got broadcast message: %s\n", rank, message);

    MPI_Finalize();  // Finalize MPI
    return 0;
}
