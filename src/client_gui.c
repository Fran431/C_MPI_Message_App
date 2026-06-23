#include "client_gui.h"

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
    printf("Available commands: @msg <rank> <message> | @diffusion <message> | @exit\n");
 
    Queue_t incoming_queue, outgoing_queue;
    initialize_queue(&incoming_queue);
    initialize_queue(&outgoing_queue);
 
    Comm_thread_data_t comm_data = { rank, "", &outgoing_queue };
    strncpy(comm_data.name, name, MAX_NAME_LEN - 1);
 
    pthread_t comm_thread, interface_thread;
    pthread_create(&comm_thread, NULL, comm_work, &comm_data);
    pthread_create(&interface_thread, NULL, interface_work, &incoming_queue);
 
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