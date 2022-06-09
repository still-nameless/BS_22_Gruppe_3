#include <stdlib.h>
#include "helper_functions.h"

struct Statement processInput(char* input)
{
    struct Statement statement;
    statement.commandExists = 0;
    statement.keyExists = 0;
    statement.valueExists = 0;

    char delim[] = " \n";
    char* command = NULL;
    command = strtok(input, delim);
    if(command != NULL) {
        strcpy(statement.command, command);
        statement.commandExists = 1;
    }

    if((strcmp(command, "QUIT") != 0 && strcmp(command, "quit") != 0) ||
       (strcmp(command, "BEG") != 0 && strcmp(command, "beg") != 0 ) ||
       (strcmp(command, "END") != 0 && strcmp(command, "end") != 0 ))
    {
        char* key = NULL;
        key = strtok(NULL, delim);
        char* value = NULL;
        if(key != NULL) {
            strcpy(statement.key, key);
            statement.keyExists = 1;
        }
        if(!strcmp(command, "PUT") || !strcmp(command, "put"))
        {
            value = strtok(NULL, "\n");
            if(value != NULL) {
                strcpy(statement.value, value);
                statement.valueExists = 1;
            }
        }
    }
    return statement;
}

void extract_key(char* msg, char* key)
{
    char delim[] = ":";
    char msg_cpy[256];
    strcpy(msg_cpy, msg);
    strtok(msg_cpy, delim);
    char* tmp = NULL;
    tmp = strtok(NULL, delim);
    strcpy(key, tmp);
}

void cut_garbage(char* msg, char* new_msg)
{
    char* new = NULL;
    new = strtok(msg, "\n");
    strcat(new, "\0");
    strcpy(new_msg, new);
}

int* createSharedMemoryForTransactions()
{
    int shmID = shmget(IPC_PRIVATE, SHMSEGSIZE, IPC_CREAT | 0777);
    int *shared_mem = (int*) shmat(shmID, 0, 0);
    return shared_mem;
}

void replaceCharactersInString(char* input, char old, char new)
{
    int i = 0;
    while (input[i] != '\0')
    {
        if(input[i] == old)
        {
            input[i] = new;
        }
        i++;
    }
}

void create_semaphore()
{
    unsigned short marker[1];
    sem_id = semget(IPC_PRIVATE, 1, IPC_CREAT|0644);
    if(sem_id == -1)
    {
        perror("semget");
        exit(1);
    }
    marker[0] = 1;
    semctl(sem_id, 1, SETALL, marker);
    enter.sem_num = leave.sem_num = 0;
    enter.sem_flg = leave.sem_flg = SEM_UNDO;
    enter.sem_op = -1;
    leave.sem_op = 1;
}