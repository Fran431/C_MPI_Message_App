#ifndef PROTOCOL_H
#define PROTOCOL_H


#define MAX_NAME_LEN      32
#define MAX_MSG_LEN      256
#define MAX_DIFF_LIST      1
#define MAX_LIST_MEMBERS 16
#define COORDINATOR 0


typedef enum message_types{
    TAG_REGISTER,
    TAG_DIRECT,
    TAG_DIFFUSION,
    TAG_DISCONNECT,
    TAG_ERROR
} msg_types_t;


typedef struct message{
    int   sender_rank;
    char  sender_name[MAX_NAME_LEN];
 
    int   receiver_rank;
    char  list_name[MAX_NAME_LEN];
 
    int   message_length;
    char  message_body[MAX_MSG_LEN];   
 
    long  timestamp;
} Message_t;


typedef struct entry{
    int  rank;
    char name[MAX_NAME_LEN];
} Client_register_protocol_t;

#endif