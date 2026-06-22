#include <mpi.h>

#include "protocol.h"
#include "coordinator.h"
#include "client.h"

int main(int argc, char **argv) {
    int rank, process_num;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &process_num);
    if (rank == COORDINATOR) {
        run_coordinator(process_num);
    } else {
    run_client(rank, process_num);
    }

    MPI_Finalize();
    return 0;
}