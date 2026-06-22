#include "client.h"

pthread_mutex_t output_mutex;


void get_register_disconnect_msg(Message_t* msg, int rank, char* name){
    memset(msg, 0, sizeof(Message_t));
    msg->sender_rank = rank;
    strncpy(msg->sender_name, name, MAX_NAME_LEN - 1);
}

void get_direct_message(Message_t *msg, int rank, const char *name, int receiver, char *text) {
    memset(msg, 0, sizeof(Message_t));
    msg->sender_rank = rank;
    strncpy(msg->sender_name, name, MAX_NAME_LEN - 1);
    msg->receiver_rank = receiver;
    strncpy(msg->message_body, text, MAX_MSG_LEN - 1);
    msg->message_length = strlen(msg->message_body);
}

void get_diffusion_message(Message_t* msg, int rank, char* name, char* text){
    memset(msg, 0, sizeof(Message_t));
    msg->sender_rank = rank;
    strncpy(msg->sender_name, name, MAX_NAME_LEN - 1);
    msg->receiver_rank = -1;
    strncpy(msg->message_body, text, MAX_MSG_LEN - 1);
    msg->message_length = strlen(msg->message_body);
}

void *comm_work(void *arg) {
    Comm_thread_data_t *data = (Comm_thread_data_t *)arg;
    char line[MAX_MSG_LEN];
    Message_t msg;
 
    while (true) {
        pthread_mutex_lock(&output_mutex);
        printf("> ");
        fflush(stdout);
        pthread_mutex_unlock(&output_mutex);
 
        if (fgets(line, sizeof(line), stdin) == NULL) {
            get_register_disconnect_msg(&msg, data->rank, data->name);
            enqueue(data->output, TAG_DISCONNECT, &msg);
            break;
        }

        line[strcspn(line, "\n")] = '\0';
 
        if (strcmp(line, "@exit") == 0) {
            get_register_disconnect_msg(&msg, data->rank, data->name);
            enqueue(data->output, TAG_DISCONNECT, &msg);
            break;

        } else if (strncmp(line, "@bcast", 6) == 0) {
            get_diffusion_message(&msg, data->rank, data->name, line + 7);
            enqueue(data->output, TAG_DIFFUSION, &msg);

        } else if (strncmp(line, "@msg", 4) == 0) {
            int receiver;
            char text[MAX_MSG_LEN];

            if (sscanf(line + 5, "%d %[^\n]", &receiver, text) == 2) {
                get_direct_message(&msg, data->rank, data->name, receiver, text);
                enqueue(data->output, TAG_DIRECT, &msg);

            } else {
                pthread_mutex_lock(&output_mutex);
                printf("Usage: @msg <rank> <texto>\n");
                pthread_mutex_unlock(&output_mutex);
            }

        } else if (strlen(line) > 0) {
            pthread_mutex_lock(&output_mutex);
            printf("Invalid command. Use @msg, @bcast o @exit.\n");
            pthread_mutex_unlock(&output_mutex);
        }
    }
    return NULL;
}


void* interface_work(void *arg) {
    Queue_t *incoming = (Queue_t *)arg;
    Msg_types_t tag;
    Message_t msg;
 
    while (blocking_dequeue(incoming, &tag, &msg)) {
        pthread_mutex_lock(&output_mutex);
        switch (tag) {
            case TAG_DIRECT:
                printf("\n[%s] %s\n", msg.sender_name, msg.message_body);
                break;
            case TAG_DIFFUSION:
                printf("\n[%s] %s\n", msg.sender_name, msg.message_body);
                break;
            default:
                break;
        }
        pthread_mutex_unlock(&output_mutex);
    }
    return NULL;
}


void run_client(int rank, int num_process) {
    (void)num_process;

    pthread_mutex_init(&output_mutex, NULL);
 
    char name[MAX_NAME_LEN];
    printf("Please write your username");
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
 
        if (!finished) {
            usleep(30);
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
