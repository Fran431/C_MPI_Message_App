#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>

#include "protocol.h"

typedef struct Node {
    Msg_types_t tag;
    Message_t msg;
    struct Node *next;
} Node_t;

typedef struct {
    Node_t *head;
    Node_t *tail;
    size_t size;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    bool closed;
} Queue_t;


void initialize_queue(Queue_t *q);

void destroy_queue(Queue_t *q);

void enqueue(Queue_t *q, Msg_types_t msg_type, Message_t* msg);

int blocking_dequeue(Queue_t* q, Msg_types_t* tag, Message_t* msg);

int not_blocking_dequeue(Queue_t *q, Msg_types_t *tag, Message_t *msg);

void queue_close(Queue_t *q);

int queue_size(Queue_t *q);

void free_node(Node_t *node);