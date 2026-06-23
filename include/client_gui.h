#ifndef CLIENT_GUI
#define CLIENT_GUI
 
#include <stdio.h>
#include <mpi.h>
#include <gtk/gtk.h>


#include "protocol.h"
#include "queue.h"

typedef struct gtk_data{
    int rank;
    char name[MAX_NAME_LEN];
    Queue_t incoming_queue;
    Queue_t outgoing_queue;
    GtkTextBuffer* chat_buffer;
    GtkWidget* chat_view;
    GtkWidget* receiver_entry;
    GtkWidget* message_entry;
} GTK_data_t;

void run_GUI_client(int rank, int num_process);
 
#endif