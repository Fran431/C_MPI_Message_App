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
    Queue_t* incoming_queue;
    Queue_t* outgoing_queue;
    GtkTextBuffer* chat_buffer;
    GtkWidget* chat_view;
    GtkWidget* receiver_entry;
    GtkWidget* message_entry;
} GTK_data_t;

extern pthread_mutex_t output_mutex;

void get_register_disconnect_msg(Message_t* msg, int rank, char* name);

void get_direct_message(Message_t *msg, int rank, const char *name, int receiver, const char *text);

void get_diffusion_message(Message_t* msg, int rank, char* name, const char* text);

void show_text(GTK_data_t *data, const char *text);

gboolean check_incoming_queue(gpointer data_gtk);

void send_pressed(gpointer user_data);

void closed_window(gpointer data_gtk);

void *gtk_work(void *arg_void);

void run_GUI_client(int rank);
 
#endif