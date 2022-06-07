#include "key_val_store.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define MAX_STORE_SIZE 10

Data_point* shared_memory;

Data_point* create_shared_memory()
{
    int shm_id;
    if((shm_id = shmget(IPC_PRIVATE, MAX_STORE_SIZE * sizeof(Data_point), SHM_R | SHM_W)) < 0)
    {
        perror("shmget");
        exit(1);
    }
    if((shared_memory = (Data_point*) shmat(shm_id, 0, 0)) < 0)
    {
        perror("shmat");
        exit(1);
    }
    for(int i=0; i<MAX_STORE_SIZE; i++)
    {
        strcpy(shared_memory[i].key, "NULL");
        strcpy(shared_memory[i].value, "NULL");
    }

    return shared_memory;
}

int put(char* key, char* value)
{
    Data_point tmp = {*key, *value};
    for(int i = 0; i < MAX_STORE_SIZE; i++)
    {
        if(strcmp(shared_memory[i].key, key) == 0)
        {
            strcpy(shared_memory[i].value, value);
            return 1;
        }
    }
    for(int i = 0; i < MAX_STORE_SIZE; i++)
    {
        if(strcmp(shared_memory[i].key, "NULL") == 0)
        {
            strcpy(shared_memory[i].key, key);
            strcpy(shared_memory[i].value, value);
            return 1;
        }
    }
    perror("maximal capacity reached!");
    return -1;
}

int get(char* key, char* res)
{
    for(int i = 0; i < MAX_STORE_SIZE; i++)
    {
        if(strcmp(shared_memory[i].key, key) == 0)
        {
            strcpy(res, shared_memory[i].value);
            return 1;
        }
    }
    strcpy(res, "key_nonexistent");
    return -1;
}

int del(char* key)
{
    for(int i = 0; i < MAX_STORE_SIZE; i++)
    {
        if(strcmp(shared_memory[i].key, key) == 0)
        {
            strcpy(shared_memory[i].key, "NULL");
            strcpy(shared_memory[i].value, "NULL");
            return 1;
        }
    }
    return -1;
}