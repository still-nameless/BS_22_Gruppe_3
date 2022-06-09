#include "key_val_store.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

Data_point* shared_memory_key_val_store;

void create_shared_memory_store()
{
    int shm_id;
    if((shm_id = shmget(IPC_PRIVATE, MAX_STORE_SIZE * sizeof(Data_point), SHM_R | SHM_W)) < 0)
    {
        perror("shmget");
        exit(1);
    }
    if((shared_memory_key_val_store = (Data_point*) shmat(shm_id, 0, 0)) < 0)
    {
        perror("shmat");
        exit(1);
    }
    for(int i=0; i<MAX_STORE_SIZE; i++)
    {
        strcpy(shared_memory_key_val_store[i].key, "NULL");
        strcpy(shared_memory_key_val_store[i].value, "NULL");
    }
}

void create_shared_memory_subs()
{
    int shm_id;
    if((shm_id = shmget(IPC_PRIVATE, MAX_STORE_SIZE * sizeof(Subs), SHM_R | SHM_W)) < 0)
    {
        perror("shmget");
        exit(1);
    }
    if((shared_memory_subs = (Subs*) shmat(shm_id, 0, 0)) < 0)
    {
        perror("shmat");
        exit(1);
    }
    for(int i = 0; i < MAX_STORE_SIZE; i++)
    {
        strcpy(shared_memory_subs[i].key, "NULL");
        for (int j = 0; j < MAX_CLIENTS; ++j)
        {
            shared_memory_subs[i].subs[j] = 0;
        }
    }
}

int put(char* key, char* value, char* msg, int msg_size)
{
    int new_entry = 1;
    for(int i = 0; i < MAX_STORE_SIZE; i++)
    {
        if(strcmp(shared_memory_key_val_store[i].key, key) == 0)
        {
            strcpy(shared_memory_key_val_store[i].value, value);
            new_entry = 0;
            break;
        }
    }
    if(new_entry)
    {
        for(int i = 0; i < MAX_STORE_SIZE; i++)
        {
            if(strcmp(shared_memory_key_val_store[i].key, "NULL") == 0)
            {
                strcpy(shared_memory_key_val_store[i].key, key);
                strcpy(shared_memory_key_val_store[i].value, value);
                break;
            }
        }
    }

    new_entry = 1;

    for(int i = 0; i < MAX_STORE_SIZE; i++)
    {
        if(strcmp(shared_memory_subs[i].key, key) == 0)
        {
            new_entry = 0;
            break;
        }
    }
    if(new_entry)
    {
        for(int i = 0; i < MAX_STORE_SIZE; i++)
        {
            if(strcmp(shared_memory_subs[i].key, "NULL") == 0)
            {
                strcpy(shared_memory_subs[i].key, key);
                break;
            }
        }
    }

    return 1;
}

int get(char* key, char* res)
{
    for(int i = 0; i < MAX_STORE_SIZE; i++)
    {
        if(strcmp(shared_memory_key_val_store[i].key, key) == 0)
        {
            strcpy(res, shared_memory_key_val_store[i].value);
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
        if(strcmp(shared_memory_key_val_store[i].key, key) == 0)
        {
            strcpy(shared_memory_key_val_store[i].key, "NULL");
            strcpy(shared_memory_key_val_store[i].value, "NULL");
            return 1;
        }
    }
    return -1;
}

int sub(char* key, int connection_descriptor)
{
    for(int i = 0; i < MAX_STORE_SIZE; i++)
    {
        if(strcmp(shared_memory_subs[i].key, key) == 0)
        {
            for(int j = 0; j < MAX_CLIENTS; j++)
            {
                if(shared_memory_subs[i].subs[j] == connection_descriptor)
                {
                    return 2;
                }
            }
            for(int j = 0; j < MAX_CLIENTS; j++)
            {
                if(shared_memory_subs[i].subs[j] == 0)
                {
                    shared_memory_subs[i].subs[j] = connection_descriptor;
                    return 1;
                }
            }
            return 1;
        }
    }
    return -1;
}

int del_sub(char* key)
{
    for(int i = 0; i < MAX_STORE_SIZE; i++)
    {
        if(strcmp(shared_memory_subs[i].key, key) == 0)
        {
            strcpy(shared_memory_subs[i].key, "NULL");
            for (int j = 0; j < MAX_CLIENTS; ++j)
            {
                shared_memory_subs[i].subs[j] = 0;
            }
            break;
        }
    }
}