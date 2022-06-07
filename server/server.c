#include "server.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "helper_functions.h"
#include "key_val_store.h"


#define BUFSIZE 1024 // Größe des Buffers
#define TRUE 1
#define PORT 5678


void handleUserInput(struct Statement *statement, int connection_descriptor, int* isRunningTransaction, int* shared_mem, int* quit);

void start_server() {

    int server_socket; // Rendevouz-Descriptor
    int connection_descriptor; // Verbindungs-Descriptor

    int* shared_mem = createSharedMemoryForTransactions();
    int isRunningTransaction = 0;

    struct sockaddr_in client; // Socketadresse eines Clients
    socklen_t client_len; // Länge der Client-Daten
    char input[BUFSIZE]; // Daten vom Client an den Server
    int bytes_read; // Anzahl der Bytes, die der Client geschickt hat


    // Socket erstellen
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0 ){
        fprintf(stderr, "failed to create socket\n");
        exit(-1);
    }

    // Socket Optionen setzen für schnelles wiederholtes Binden der Adresse
    int option = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const void *) &option, sizeof(int));

    // Socket binden
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);
    int brt = bind(server_socket, (struct sockaddr *) &server, sizeof(server));
    if (brt < 0 ){
        fprintf(stderr, "socket konnte nicht gebunden werden\n");
        exit(-1);
    }


    // Socket lauschen lassen
    int lrt = listen(server_socket, 5);
    if (lrt < 0 ){
        fprintf(stderr, "socket konnte nicht listen gesetzt werden\n");
        exit(-1);
    }

    while (1) {
        printf("awaiting connenction\n");
        // Verbindung eines Clients wird entgegengenommen
        connection_descriptor = accept(server_socket, (struct sockaddr *) &client, &client_len);

        int running = 1;
        createNewProcess(&running, connection_descriptor);

        while (running) {
            // Lesen von Daten, die der Client schickt
            bytes_read = read(connection_descriptor, input, BUFSIZE);
            replaceCharactersInString(input, '\r', '\0');
            printf("input -> %s\n", input);
            struct Statement statement = (struct Statement) processInput(input);

            handleUserInput(&statement, connection_descriptor, &isRunningTransaction, shared_mem, &running);
        }
    }
}

void handleUserInput(struct Statement *statement, int connection_descriptor, int* isRunningTransaction, int* shared_mem, int* quit)
{
    if((*shared_mem && *isRunningTransaction == 0) && strcmp(statement->command, "QUIT") != 0 && strcmp(statement->command, "quit") != 0)
    {
        char msg[] = "ERROR: there is an active transaction!\n";
        write(connection_descriptor, msg, sizeof(msg));
        //continue;
    }
    else if((strcmp(statement->command, "BEG") == 0 || strcmp(statement->command, "beg") == 0))
    {
        *shared_mem = 1;
        *isRunningTransaction = 1;
        char msg[] = "TRANSACTION STARTED\n";
        write(connection_descriptor, msg, sizeof(msg));
    }
    else if((strcmp(statement->command, "END") == 0 || strcmp(statement->command, "end") == 0))
    {
        if(*shared_mem)
        {
            *shared_mem = 0;
            *isRunningTransaction = 0;
            char msg[] = "TRANSACTION ENDED\n";
            write(connection_descriptor, msg, sizeof(msg));
        }
        else
        {
            char msg[] = "ERROR: no active transaction";
            write(connection_descriptor, msg, sizeof(msg));
        }
    }
    else if(statement->keyExists && statement->valueExists && (strcmp(statement->command, "PUT") == 0 || strcmp(statement->command, "put") == 0)) {
        char msg[1024] = "PUT:";
        strcat(msg, statement->key);
        strcat(msg, ":");
        strcat(msg, statement->value);
        strcat(msg, "\n");
        write(connection_descriptor, msg, sizeof(msg));
        put(statement->key, statement->value);
    }
    else if(statement->keyExists && (strcmp(statement->command, "GET") == 0 || strcmp(statement->command, "get") == 0)) {
        int res;
        char* result;
        get(statement->key, result);
        char msg[1024] = "GET:";
        if(result != NULL) {
            strcat(msg, statement->key);
            strcat(msg, ":");
            strcat(msg, result);
            strcat(msg, "\n");
            free(result);
        }
        else {
            strcat(msg, statement->key);
            strcat(msg, ":key_nonexistent\n");
        }
        write(connection_descriptor, msg, sizeof(msg));
    }
    else if(statement->keyExists && (strcmp(statement->command, "DEL") == 0 || strcmp(statement->command, "del") == 0)) {
        int res;
        res = del(statement->key);
        char msg[50] = "DEL:";
        if(res == 1) {
            strcat(msg, statement->key);
            strcat(msg, ":key_deleted\n");
        }
        else {
            strcat(msg, statement->key);
            strcat(msg, ":key_nonexistent\n");
        }
        write(connection_descriptor, msg, sizeof(msg));
    }
    else if(strcmp(statement->command, "QUIT") == 0 || strcmp(statement->command, "quit") == 0) {
        if(*isRunningTransaction)
        {
            *shared_mem = 0;
            char msg[] = "TRANSACTION ENDED\n";
            write(connection_descriptor, msg, sizeof(msg));
        }
        *quit = 0;
        close(connection_descriptor);
    }
    else
    {
        char msg[] = "There is no such command like this\n";
        write(connection_descriptor, msg, sizeof(msg));
    }
}

int createNewProcess(int* quit, int connection_descriptor)
{
    int pid = fork();
    if(pid < 0)
    {
        printf("failed to create child process\n");
    }
    else if(pid == 0)
    {
        *quit = 0;
        close(connection_descriptor);
    }
    return pid;
}
