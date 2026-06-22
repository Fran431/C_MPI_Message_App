#include "../include/coordinator.h"



void update_log(char* text){
    FILE *file = fopen("../logs/coordinator_log.log", "a");
    if (file) {
        fprintf(file, "%s\n", text);
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
        snprintf(line, sizeof(line), "REGISTERED Rank= %d Name= %s", sender, table[sender].name);
        update_log(line);
        printf("Coordinator registered: rank %d -> %s\n", sender, table[sender].name);
    }
}

void route_messages(UserEntry_t* table, int client_num, int num_process){
    int active_clients = client_num;
    while (active_clients > 0) {
        Message_t msg;
        MPI_Status status;
        MPI_Recv(&msg, sizeof(Message_t), MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG,
                 MPI_COMM_WORLD, &status);
 
        switch (status.MPI_TAG) {
 
            case TAG_DIRECT: {
                int receiver = msg.receiver_rank;
                char line[256];
 
                if (table[receiver].active) {
                    MPI_Send(&msg, sizeof(Message_t), MPI_BYTE, receiver,
                             TAG_DIRECT, MPI_COMM_WORLD);
                    snprintf(line, sizeof(line),
                             "Direct message from rank %d (%s) to rank %d (%s)",
                             msg.sender_rank, msg.sender_name, receiver, table[receiver].name);
                } else{
                snprintf(line, sizeof(line),
                             "Failed to send direct message from %d (%s) to rank %d (%s)",
                             msg.sender_rank, msg.sender_name, receiver, table[receiver].name);     
                }
                update_log(line);
                break;
            }
 
            case TAG_DIFFUSION: {
                for (int receiver = 1; receiver < num_process; receiver++) {
                    if (receiver != msg.sender_rank && table[receiver].active) {
                        MPI_Send(&msg, sizeof(Message_t), MPI_BYTE, receiver,
                                 TAG_DIFFUSION, MPI_COMM_WORLD);
                    }
                }
                char line[128];
                snprintf(line, sizeof(line), "Diffusion message sent from %d (%s)",
                         msg.sender_rank, msg.sender_name);
                update_log(line);
                break;
            }
 
            case TAG_DISCONNECT: {
                int sender = msg.sender_rank;
                table[sender].active = 0;
                active_clients--;
 
                char line[128];
                snprintf(line, sizeof(line), "rank=%d (%s) disconnected. Clients left: %d",
                         sender, table[sender].name, active_clients);
                update_log(line);
                break;
            }
        }
    }
}

void run_coordinator(int num_process){

    if (num_process < 2){
        fprintf(stderr, "Coordinator needs at least 1 client connected.");
        return;
    }

    int client_num = num_process - 1;
    UserEntry_t table[num_process];

        for (int i = 0; i < num_process; i++) {
        table[i].active = false;
        table[i].name[0] = '\n';
    }

    printf("Coordinator waiting to register clients...\n");

    register_clients(table, client_num);

    route_messages(table, client_num, num_process);

    update_log("All clients disconnected.");
}



