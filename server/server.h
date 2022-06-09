#ifndef BS_22_GRUPPE_3_SERVER_H
#define BS_22_GRUPPE_3_SERVER_H

#define MAX_KEY_SIZE 256
#define MAX_MSG_SIZE 256
#define MAX_CLIENTS 1024
#define ENDLESSLOOP 1

typedef struct Text_message
{
    long mtype;
    char mtext[MAX_MSG_SIZE];
    //char mkey[MAX_KEY_SIZE];
} Text_message;

_Noreturn void start_server();

void createNewProcess(int* quit, int connection_descriptor, int* message_manager, int message_id);



#endif //BS_22_GRUPPE_3_SERVER_H
