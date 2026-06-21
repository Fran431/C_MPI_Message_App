#include "../include/coordinator.h"
#include <stdio.h>



void register_clients(UserEntry_t* table, int client_num){
    int registered = 0;
    while (registered < client_num) {
        Message_t msg;
        MPI_Status status;
        MPI_Recv(&msg, sizeof(Message_t), MPI_BYTE, MPI_ANY_SOURCE, TAG_REGISTER,
                 MPI_COMM_WORLD, &status);
 
        int sender = msg.sender_rank;
        strncpy(table[sender - 1].name, msg.sender_name, MAX_NAME_LEN);
        table[sender - 1].active = true;
        registered++;
    }
}


void run_coordinator(int process_num){

    if (process_num < 2){
        fprintf(stderr, "Coordinator needs at least 1 client connected.");
        return;
    }

    int client_num = process_num - 1;
    UserEntry_t* table[client_num];

        for (int i = 0; i < client_num; i++) {
        table[i]->active = false;
        table[i]->name[0] = "";
    }

    printf("Coordinator waiting to register clients...\n");

    register_clients(table, client_num);

}



