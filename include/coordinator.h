#ifndef COORDINATOR_H
#define COORDINATOR_H

#include <stdbool.h>

#include "../include/protocol.h"

typedef struct {
    char name[MAX_NAME_LEN];
    bool active;   
} UserEntry_t;

void run_coordinator(int process_num);

void register_clients(UserEntry_t* table);

#endif