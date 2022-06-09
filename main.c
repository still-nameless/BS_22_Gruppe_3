#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "server/server.h"
#include "key_val_store/key_val_store.h"
#include "helper_functions/helper_functions.h"
#include <string.h>
#include <sys/wait.h>



int main() {

    printf("starting server..!\n");

    create_shared_memory();
    start_server();



    return 0;
}

/*Data_point* shared_memory = create_shared_memory();

    char key[256];
    char value[256];

    int result;

    result = put("key_1", "value_1");
    result = get("key_1", value);
    if(result)
    {
        printf("key_1:%s\n", value);
    }
    put("key_2", "value_2");
    result = get("key_2", value);
    if(result)
    {
        printf("key_2:%s\n", value);
    }
    result = put("key_1", "value_5");
    result = get("key_1", value);
    if(result)
    {
        printf("key_1:%s\n", value);
    }*/

/*int pid = fork();
    if(pid < 0)
    {
        perror("fork\n");
        exit(1);
    }
    else if(pid == 0)
    {
        printf("inside child (sounds wrong)\n");
        for(int i=0; i<10; i++)
        {
            strcpy(shared_memory[i].key, "child child child\n");
        }
        printf("finished doing stuff\n");
        exit(pid);
    }
    else if(pid > 0)
    {
        wait(NULL);
        printf("inside parent (sounds wrong)\n");
        for(int i=0; i<10; i++)
        {
            printf("%d -> %s", i+1, shared_memory[i].key);
        }
    }*/