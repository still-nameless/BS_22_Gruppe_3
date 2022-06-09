#ifndef BS_22_GRUPPE_3_HELPER_FUNCTIONS_H
#define BS_22_GRUPPE_3_HELPER_FUNCTIONS_H

#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#define SHMSEGSIZE sizeof(int)

struct Statement{
    char command[5];
    char key[56];
    char value[1024];
    int commandExists;
    int keyExists;
    int valueExists;
};

int sem_id;
struct sembuf enter, leave;

struct Statement processInput(char* input);
int* createSharedMemoryForTransactions();
void replaceCharactersInString(char* input, char old, char new);
void extract_key(char* msg, char* key);
void cut_garbage(char* msg, char* new_msg);
void create_semaphore();

#endif //BS_22_GRUPPE_3_HELPER_FUNCTIONS_H
