#include <mpi>

#include "../include/protocol.h"

int main(int argc, char **argv) {
    int rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == COORDINATOR) {
        run_coordinator();
    } else {
        run_client(rank);
    }
    
    MPI_Finalize();
    return 0;
}