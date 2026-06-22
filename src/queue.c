#include "queue.h"

void initialize_queue(Queue_t *q)
{
    q->head = q->tail = NULL;
    q->size = 0;
    q->closed = false;
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->not_empty, NULL);
}

void destroy_queue(Queue_t *q)
{
    pthread_mutex_lock(&q->lock);
    Node_t *current = q->head;
    while (current)
    {
        Node_t *next = current->next;
        free(current);
        current = next;
    }
    q->head = q->tail = NULL;
    q->size = 0;
    pthread_mutex_unlock(&q->lock);
    pthread_mutex_destroy(&q->lock);
    pthread_cond_destroy(&q->not_empty);
}


void enqueue(Queue_t *q, Msg_types_t tag, Message_t* msg)
{
    Node_t *node = (Node_t *)malloc(sizeof(Node_t));
    node->tag = tag;
    node->msg = *msg;
    node->next = NULL;
    pthread_mutex_lock(&q->lock);
    if (q->tail)
    {
        q->tail->next = node;
    }
    else
    {
        q->head = node;
    }
    q->tail = node;
    q->size++;
    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->lock);
}

int blocking_dequeue(Queue_t* q, Msg_types_t* tag, Message_t* msg)
{
    pthread_mutex_lock(&q->lock);
 
    while (q->head == NULL && !q->closed) {
        pthread_cond_wait(&q->not_empty, &q->lock);
    }
 
    if (q->head == NULL) {
        pthread_mutex_unlock(&q->lock);
        return 0;
    }
 
    Node_t *node = q->head;
    *tag = node->tag;
    *msg = node->msg;
    q->head = node->next;
    if (q->head == NULL) {
        q->tail = NULL;
    }
    pthread_mutex_unlock(&q->lock);
    free(node);
    return 1;

}

int not_blocking_dequeue(Queue_t *q, Msg_types_t *tag, Message_t *msg) {
    pthread_mutex_lock(&q->lock);
 
    if (q->head == NULL) {
        pthread_mutex_unlock(&q->lock);
        return 0;
    }
 
    Node_t *node = q->head;
    *tag = node->tag;
    *msg = node->msg;
    q->head = node->next;
    if (q->head == NULL) {
        q->tail = NULL;
    }
    pthread_mutex_unlock(&q->lock);
    free(node);
    return 1;
}


void queue_close(Queue_t *q)
{
    pthread_mutex_lock(&q->lock);
    q->closed = true;
    pthread_cond_broadcast(&q->not_empty);
    pthread_mutex_unlock(&q->lock);
}

int queue_size(Queue_t *q)
{
    pthread_mutex_lock(&q->lock);
    int s = q->size;
    pthread_mutex_unlock(&q->lock);
    return s;
}

void free_node(Node_t *node)
{
    free(node);
}
