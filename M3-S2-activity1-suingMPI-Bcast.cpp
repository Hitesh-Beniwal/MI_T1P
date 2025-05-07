#include <mpi.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv) {
    int rank;
    char message[100];

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        strcpy(message, "Hello World!");
    }

    // Broadcast message to all processes
    MPI_Bcast(message, 100, MPI_CHAR, 0, MPI_COMM_WORLD);

    // All processes print the message
    printf("Process %d received message via broadcast: %s\n", rank, message);

    MPI_Finalize();
    return 0;
}
