#ifndef CLIENT_GUI
#define CLIENT_GUI
 
#include <stdio.h>
#include <mpi.h>


#include "protocol.h"
#include "queue.h"

void run_GUI_client(int rank, int num_process);
 
#endif