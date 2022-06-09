//
// Created by me on 5/29/22.
//
#ifndef BS_22_GRUPPE_3_KEY_VAL_STORE_H
#define BS_22_GRUPPE_3_KEY_VAL_STORE_H

#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#define MAX_KEY_SIZE 256
#define MAX_VALUE_SIZE 256
#define MAX_STORE_SIZE 1024
#define MAX_CLIENTS 1024

typedef struct Data_point
{
    char key[MAX_STORE_SIZE];
    char value[MAX_VALUE_SIZE];
} Data_point;

typedef struct Subs
{
    char key[MAX_STORE_SIZE];
    int subs[MAX_CLIENTS];
} Subs;

Data_point* shared_memory_key_val_store;
void create_shared_memory_store();

Subs* shared_memory_subs;
void create_shared_memory_subs();

int put(char* key, char* value, char* msg, int msg_size);
int get(char* key, char* res);
int del(char* key);
int sub(char* key, int connection_descriptor);
int del_sub(char* key);

#endif //BS_22_GRUPPE_3_KEY_VAL_STORE_H
