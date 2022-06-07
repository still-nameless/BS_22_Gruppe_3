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

typedef struct Data_point
{
    char key[MAX_KEY_SIZE];
    char value[MAX_VALUE_SIZE];
} Data_point;

Data_point* create_shared_memory();

int put(char* key, char* value);
int get(char* key, char* res);
int del(char* key);

#endif //BS_22_GRUPPE_3_KEY_VAL_STORE_H
