#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <mpi.h>


#include "protocol.h"
#include "queue.h"

extern pthread_mutex_t output_mutex;

typedef struct Comm_thread{
    int rank;
    char name[MAX_NAME_LEN];
    Queue_t *output;
} Comm_thread_data_t;

void get_register_disconnect_msg(Message_t* msg, int rank, char* name);

void get_direct_message(Message_t *msg, int rank, const char *name, int receiver, char *text);

void get_diffusion_message(Message_t* msg, int rank, char* name, char* text);

void *comm_work(void *arg);

void* interface_work(void *arg);

void run_client(int rank);

#endif