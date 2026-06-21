#include "../include/coordinator.h"
#include <stdio.h>

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

    register_clients(table);
    
}

