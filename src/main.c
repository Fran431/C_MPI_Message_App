#include <stdio.h>
#include <mpi.h>
#include "protocol.h"
#include "coordinator.h"
 

#ifdef GUI
    #include "client_gui.h"
#else
    #include "client_cli.h"
#endif
 
int main(int argc, char **argv) {
    int rank, num_process, threads_obtained;
 
    MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &threads_obtained);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_process);
 

 
    if (num_process < 2) {
        if (rank == COORDINATOR) {
            fprintf(stderr, "At least 2 processes are needed.\n");
        }
        MPI_Finalize();
        return 1;
    }
 
    if (rank == COORDINATOR) {
        run_coordinator(num_process);
    } else {
#ifdef GUI
        run_GUI_client(rank);
#else
        run_client(rank);
#endif
    }
 
    MPI_Finalize();
    return 0;
}
 
