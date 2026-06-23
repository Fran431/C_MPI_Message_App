#include "client_gui.h"







void *hilo_gtk(void *arg_void) {
    GTK_data_t *data = (GTK_data_t *)arg_void;
 
    gtk_init(NULL, NULL);
 
    char title[64];
    snprintf(title, sizeof(title), "Client - %s (rank %d)", data->name, data->rank);
    
    // Create a window, apparently, almost always it should be a top level one.
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), title);
    gtk_window_set_default_size(GTK_WINDOW(ventana), 480, 400);
    g_signal_connect(ventana, "destroy", G_CALLBACK(on_cerrar_ventana), ctx);
    //Create a box
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_container_set_border_width(GTK_CONTAINER(main_box), 8);
    gtk_container_add(GTK_CONTAINER(ventana), main_box);
    
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
    gtk_entry_set_placeholder_text(GTK_ENTRY(data->receiver_entry), "Send message to...");
    gtk_entry_set_width_chars(GTK_ENTRY(data->receiver_entry), 14);
    gtk_box_pack_start(GTK_BOX(send_box), data->receiver_entry, FALSE, FALSE, 0);
    
    //Text entry to write message
    data->message_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(data->message_entry), "Write a message...");
    gtk_box_pack_start(GTK_BOX(send_box), data->message_entry, TRUE, TRUE, 0);
    
    //To send message
    GtkWidget *send_button = gtk_button_new_with_label("Send");
    gtk_box_pack_start(GTK_BOX(send_box), send_button, FALSE, FALSE, 0);
 
    g_signal_connect(send_button, "clicked", G_CALLBACK(send_pressed), ctx);
    g_signal_connect(data->message_entry, "activate", G_CALLBACK(send_pressed), ctx);
 
    //g_timeout_add(INTERVALO_POLL_GUI_MS, revisar_cola_entrada, ctx);
    
    gtk_widget_show_all(window);
    //This is to keep the event loop going
    gtk_main(); 
 
    return NULL;
}




void run_GUI_client(int rank, int num_process) {
    (void)num_process;

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
    strncpy(data_gtk.name, name, MAX_NAME - 1);
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
 
    pthread_join(comm_thread, NULL);
    pthread_join(interface_thread, NULL);

    pthread_mutex_destroy(&output_mutex);
    destroy_queue(&incoming_queue);
    destroy_queue(&outgoing_queue);
 
    printf("Disconnected. Bye, %s.\n", name);
}