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

    create_shared_memory_store();
    create_shared_memory_subs();
    create_semaphore();
    start_server();



    return 0;
}

