#include "key_val_store.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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
        for(int j=0; j<MAX_STORE_SIZE; j++)
        {
            shared_memory[i].subs[j] = 0;
        }
    }

    return shared_memory;
}

int put(char* key, char* value, char* msg, int msg_size)
{
    Data_point tmp = {*key, *value};
    for(int i = 0; i < MAX_STORE_SIZE; i++)
    {
        if(strcmp(shared_memory[i].key, key) == 0)
        {
            strcpy(shared_memory[i].value, value);
            for(int j = 0; j < MAX_STORE_SIZE; j++)
            {
                if(shared_memory[i].subs[j] != 0)
                {

                    int written_bytes = write(shared_memory[i].subs[j], msg, msg_size);
                    printf("written bytes -> %d", written_bytes);
                    return 1;
                }
            }
            return 2;
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

int sub(char* key, int connection_descriptor)
{
    for(int i = 0; i < MAX_STORE_SIZE; i++)
    {
        if(strcmp(shared_memory[i].key, key) == 0)
        {
            for(int j = 0; j < MAX_STORE_SIZE; j++)
            {
                if(shared_memory[i].subs[j] == connection_descriptor)
                {
                    return 2;
                }
            }
            for(int j = 0; j < MAX_STORE_SIZE; j++)
            {
                if(shared_memory[i].subs[j] == 0)
                {
                    shared_memory[i].subs[j] = connection_descriptor;
                    return 1;
                }
            }
            return 1;
        }
    }
    return -1;
}