#include "../include/coordinator.h"
#include <stdio.h>


void update_log(char* text){
    FILE *file = fopen("../logs/coordinator_log.log", "a");
    if (file) {
        fprintf(file, "[%ld] %s\n", text);
        fclose(file);
    }
}

void register_clients(UserEntry_t* table, int client_num){
    int registered = 0;
    while (registered < client_num) {
        Message_t msg;
        MPI_Status status;
        MPI_Recv(&msg, sizeof(Message_t), MPI_BYTE, MPI_ANY_SOURCE, TAG_REGISTER,
                 MPI_COMM_WORLD, &status);
 
        int sender = msg.sender_rank;
        strncpy(table[sender].name, msg.sender_name, MAX_NAME_LEN);
        table[sender].active = true;
        registered++;
        
        char line[128];
        snprintf(line, sizeof(line), "REGISTERED Rank= %d Name= %s", r, table[sender].name);
        update_log(line);
        printf("Coordinator registered: rank %d -> %s\n", r, table[sender].name);
    }
}



void run_coordinator(int process_num){

    if (process_num < 2){
        fprintf(stderr, "Coordinator needs at least 1 client connected.");
        return;
    }

    int client_num = process_num - 1;
    UserEntry_t* table[process_num];

        for (int i = 0; i < process_num; i++) {
        table[i]->active = false;
        table[i]->name[0] = "";
    }

    printf("Coordinator waiting to register clients...\n");

    register_clients(table, client_num);

    route_messages(client_num);

}



