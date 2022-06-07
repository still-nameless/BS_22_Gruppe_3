#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include "helper_functions.h"
#define SHMSEGSIZE sizeof(int)

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
            value = strtok(NULL, delim);
            if(value != NULL) {
                strcpy(statement.value, value);
                statement.valueExists = 1;
            }
        }
    }
    return statement;
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