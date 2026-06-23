#include "client_gui.h"

pthread_mutex_t output_mutex;

void get_register_disconnect_msg(Message_t* msg, int rank, char* name){
    memset(msg, 0, sizeof(Message_t));
    msg->sender_rank = rank;
    strncpy(msg->sender_name, name, MAX_NAME_LEN - 1);
}

void get_direct_message(Message_t *msg, int rank, const char *name, int receiver, const char *text) {
    memset(msg, 0, sizeof(Message_t));
    msg->sender_rank = rank;
    strncpy(msg->sender_name, name, MAX_NAME_LEN - 1);
    msg->receiver_rank = receiver;
    strncpy(msg->message_body, text, MAX_MSG_LEN - 1);
    msg->message_length = strlen(msg->message_body);
}

void get_diffusion_message(Message_t* msg, int rank, char* name, const char* text){
    memset(msg, 0, sizeof(Message_t));
    msg->sender_rank = rank;
    strncpy(msg->sender_name, name, MAX_NAME_LEN - 1);
    msg->receiver_rank = -1;
    strncpy(msg->message_body, text, MAX_MSG_LEN - 1);
    msg->message_length = strlen(msg->message_body);
}


void ask_username(char *name, int rank) {
    //Shows small window with buttons
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Connect",
        NULL,
        GTK_DIALOG_MODAL,
        "_Connect", GTK_RESPONSE_OK,
        NULL);

    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *label = gtk_label_new("Enter your username:");
    GtkWidget *entry = gtk_entry_new();
    //Pressing enter should push the button
    gtk_entry_set_activates_default(GTK_ENTRY(entry), TRUE);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);

    gtk_box_pack_start(GTK_BOX(content), label, FALSE, FALSE, 6);
    gtk_box_pack_start(GTK_BOX(content), entry, FALSE, FALSE, 6);
    gtk_widget_show_all(dialog);

    gtk_dialog_run(GTK_DIALOG(dialog));   /* blocks until "Connect" is pressed */

    const char *text = gtk_entry_get_text(GTK_ENTRY(entry));
    if (strlen(text) == 0) {
        snprintf(name, MAX_NAME_LEN, "User-%d", rank);
    } else {
        strncpy(name, text, MAX_NAME_LEN - 1);
        name[MAX_NAME_LEN - 1] = '\0';
    }

    gtk_widget_destroy(dialog);
}

void show_text(GTK_data_t *data, const char *text) {
    //This iterates over a gtk buffer
    GtkTextIter end;
    //Position the index at the end
    gtk_text_buffer_get_end_iter(data->chat_buffer, &end);
    //Inserts the text
    gtk_text_buffer_insert(data->chat_buffer, &end, text, -1);
    //Adds newline at end of inserted text
    gtk_text_buffer_insert(data->chat_buffer, &end, "\n", -1);
    //Creates mark at end position
    GtkTextMark *end_mark = gtk_text_buffer_create_mark(data->chat_buffer, NULL, &end, FALSE);
    //scrolls to new mark (so that text remains on screen) 
    gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(data->chat_view), end_mark);
}


gboolean check_incoming_queue(gpointer data_gtk) {
    GTK_data_t *data = (GTK_data_t *)data_gtk;
    Msg_types_t tag;
    Message_t msg;
    char line[MAX_MSG_LEN + MAX_NAME_LEN + 32];
 
    while (not_blocking_dequeue(data->incoming_queue, &tag, &msg)) {
        switch (tag) {
            case TAG_DIRECT:
                snprintf(line, sizeof(line), "[%s] %s", msg.sender_name, msg.message_body);
                show_text(data, line);
                break;
            case TAG_DIFFUSION:
                snprintf(line, sizeof(line), "[%s @everyone] %s", msg.sender_name, msg.message_body);
                show_text(data, line);
                break;
            default:
                break;
        }
    }
    //When no more things are left in queue
    return TRUE;
}



//gpointer is a pointer but used by gtk functions
void send_pressed(GtkWidget *widget, gpointer data_gtk) {
    (void)widget; 
    GTK_data_t *data = (GTK_data_t *)data_gtk;
 
    const char *receiver_text= gtk_entry_get_text(GTK_ENTRY(data->receiver_entry));
    const char *message_text = gtk_entry_get_text(GTK_ENTRY(data->message_entry));
 
    if (strlen(message_text) == 0) {
        return;
    }
 
    Message_t msg;
    char line[MAX_MSG_LEN + MAX_NAME_LEN + 32];
 
    if (strlen(receiver_text) == 0) {
        get_diffusion_message(&msg, data->rank, data->name, message_text);
        enqueue(data->outgoing_queue, TAG_DIFFUSION, &msg);
        snprintf(line, sizeof(line), "@todos %s", message_text);
        //show_text(data, line);
    } else {
        int dest = atoi(receiver_text);
        if (dest <= 0 || dest  == data->rank) {
            snprintf(line, sizeof(line), "Invalid receiver rank.");
            show_text(data, line);
            return;
        }
        get_direct_message(&msg, data->rank, data->name, dest, message_text);
        enqueue(data->outgoing_queue, TAG_DIRECT, &msg);
        snprintf(line, sizeof(line), "@%d %s", dest, message_text);
    }
 
    show_text(data, line);
    //To "clean" text inside data entry widget
    gtk_entry_set_text(GTK_ENTRY(data->message_entry), "");
    //So that keyboard writes here, in data entry widget
    gtk_widget_grab_focus(data->message_entry);
}

void closed_window(GtkWidget *widget, gpointer data_gtk) {
    (void)widget; 
    GTK_data_t *data = (GTK_data_t *)data_gtk;
 
    Message_t msg;
    get_register_disconnect_msg(&msg, data->rank, data->name);
    enqueue(data->outgoing_queue, TAG_DISCONNECT, &msg);
    //Returns from the main loop(main gtk loop)
    gtk_main_quit();
}




void *gtk_work(void *arg_void) {
    GTK_data_t *data = (GTK_data_t *)arg_void;
 
    gtk_init(NULL, NULL);
 
    ask_username(data->name, data->rank); 

    char title[64];
    snprintf(title, sizeof(title), "Client - %s (rank %d)", data->name, data->rank);
    
    // Create a window, apparently, almost always it should be a top level one.
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), title);
    gtk_window_set_default_size(GTK_WINDOW(window), 480, 400);
    //Handle window closing
    g_signal_connect(window, "destroy", G_CALLBACK(closed_window), data);
    //Create a box
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_container_set_border_width(GTK_CONTAINER(main_box), 8);
    gtk_container_add(GTK_CONTAINER(window), main_box);
    
    //Usually arguments should be NULL, so that adjustments are set for me (yey)
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_vexpand(scroll, TRUE);
    data->chat_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(data->chat_view), FALSE);
    
    //Avoids text overflow (off screen text that will need scrolling ugh)
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(data->chat_view), GTK_WRAP_WORD_CHAR);
    data->chat_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(data->chat_view));

    gtk_container_add(GTK_CONTAINER(scroll), data->chat_view);
    gtk_box_pack_start(GTK_BOX(main_box), scroll, TRUE, TRUE, 0);
 
    GtkWidget *send_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_box_pack_start(GTK_BOX(main_box), send_box, FALSE, FALSE, 0);
    
    //Text entry to tell which person will receive the message
    data->receiver_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(data->receiver_entry), "Send a message to...");
    gtk_entry_set_width_chars(GTK_ENTRY(data->receiver_entry), 14);
    gtk_box_pack_start(GTK_BOX(send_box), data->receiver_entry, FALSE, FALSE, 0);
    
    //Text entry to write message
    data->message_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(data->message_entry), "Write a message...");
    gtk_box_pack_start(GTK_BOX(send_box), data->message_entry, TRUE, TRUE, 0);
    
    //To send message
    GtkWidget *send_button = gtk_button_new_with_label("Send");
    gtk_box_pack_start(GTK_BOX(send_box), send_button, FALSE, FALSE, 0);
    
    //When the send button is pressed, a function is called to handle it
    g_signal_connect(send_button, "clicked", G_CALLBACK(send_pressed), data);
    g_signal_connect(data->message_entry, "activate", G_CALLBACK(send_pressed), data);

    g_timeout_add(100, check_incoming_queue, data);
    
    gtk_widget_show_all(window);
    //This is to keep the event loop going
    gtk_main(); 
 
    return NULL;
}




void run_GUI_client(int rank) {

    pthread_mutex_init(&output_mutex, NULL);
 
    char name[MAX_NAME_LEN];
    printf("Please write your username\n> ");
    fflush(stdout);
    if (fgets(name, MAX_NAME_LEN, stdin) == NULL) {
        snprintf(name, MAX_NAME_LEN, "User-%d", rank);
    }
    name[strcspn(name, "\n")] = '\0';
    if (strlen(name) == 0) {
        snprintf(name, MAX_NAME_LEN, "User-%d", rank);
    }
 
    Message_t register_msg;
    get_register_disconnect_msg(&register_msg, rank, name);
    MPI_Send(&register_msg, sizeof(Message_t), MPI_BYTE, COORDINATOR,
              TAG_REGISTER, MPI_COMM_WORLD);
 
    printf("Connected \"%s\" rank = %d.\n", name, rank);

    //printf("Available commands: @msg <rank> <message> | @diffusion <message> | @exit\n");
 
    Queue_t incoming_queue, outgoing_queue;
    initialize_queue(&incoming_queue);
    initialize_queue(&outgoing_queue);
 
    //Comm_thread_data_t comm_data = { rank, "", &outgoing_queue };
    //strncpy(comm_data.name, name, MAX_NAME_LEN - 1);

    GTK_data_t data_gtk;
    data_gtk.rank = rank;
    strncpy(data_gtk.name, name, MAX_NAME_LEN - 1);
    data_gtk.incoming_queue = &incoming_queue;
    data_gtk.outgoing_queue = &outgoing_queue;
 
    pthread_t thread_gtk;
    pthread_create(&thread_gtk, NULL, gtk_work, &data_gtk);
 
    int finished = 0;
    while (!finished) {
        
        Msg_types_t outgoing_tag;
        Message_t outgoing_msg;
        while (not_blocking_dequeue(&outgoing_queue, &outgoing_tag, &outgoing_msg)) {
            MPI_Send(&outgoing_msg, sizeof(Message_t), MPI_BYTE, COORDINATOR,
                      outgoing_tag, MPI_COMM_WORLD);
            if (outgoing_tag == TAG_DISCONNECT) {
                finished = 1;
            }
        }
        
        int new_message;
        MPI_Status status;
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &new_message, &status);
        if (new_message) {
            Message_t incoming_msg;
            MPI_Recv(&incoming_msg, sizeof(Message_t), MPI_BYTE, status.MPI_SOURCE,
                      status.MPI_TAG, MPI_COMM_WORLD, &status);
            enqueue(&incoming_queue, (Msg_types_t)status.MPI_TAG, &incoming_msg);
        }
    }
 
    queue_close(&incoming_queue);
 
    pthread_join(thread_gtk, NULL);

    pthread_mutex_destroy(&output_mutex);
    destroy_queue(&incoming_queue);
    destroy_queue(&outgoing_queue);
 
    printf("Disconnected. Bye, %s.\n", name);
}