#include "server.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/msg.h>
#include <netinet/in.h>
#include <signal.h>
#include <fcntl.h>
#include "helper_functions.h"
#include "key_val_store.h"
#include <errno.h>


#define BUFSIZE 1024 // Größe des Buffers
#define TRUE 1
#define PORT 5678

int sockets[1024] = {0};


void handleUserInput(struct Statement *statement, int connection_descriptor, int* isRunningTransaction, int* shared_mem, int* quit, int msg_id);
void handle_messages(int message_id);

_Noreturn void start_server() {

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
    int flags = fcntl(server_socket, F_GETFL);
    fcntl(server_socket, F_SETFL, flags | O_NONBLOCK);

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

    int message_manager = -1;
    int msg_id = msgget(0, IPC_CREAT|0666);
    int running = 1;

    while (1) {
        //printf("awaiting connenction\n");
        // Verbindung eines Clients wird entgegengenommen
        connection_descriptor = accept(server_socket, (struct sockaddr *) &client, &client_len);
        if (connection_descriptor == -1)
        {
            if (errno == EWOULDBLOCK)
            {
                running = 0;
                handle_messages(msg_id);
            }
            else
            {
                perror("error when accepting connection");

                //printf(errno);
                exit(1);
            }
        }
        else
        {
            printf("client found\n");

            for (int i = 0; i < MAX_CLIENTS; ++i)
            {
                if(sockets[i] == 0)
                {
                    sockets[i] = connection_descriptor;
                    break;
                }
            }

            createNewProcess(&running, connection_descriptor, &message_manager, msg_id);
        }


        //("message manager -> %d\n", message_manager);

        while (running) {
            //char * text = "child in while";
            //printf("%s", text);
            printf("awaiting input\n");
            bytes_read = read(connection_descriptor, input, BUFSIZE);
            replaceCharactersInString(input, '\r', '\0');
            //printf("input -> %s\n", input);
            struct Statement statement = (struct Statement) processInput(input);
            semop(sem_id, &enter, 1);
            handleUserInput(&statement, connection_descriptor, &isRunningTransaction, shared_mem, &running, msg_id);
            semop(sem_id, &leave, 1);
        }
    }
}

void createNewProcess(int* running, int connection_descriptor, int* message_manager, int message_id)
{
    int pid = fork();
    if(pid < 0)
    {
        //printf("failed to create child process\n");
    }
    else if(pid > 0)
    {
        *running = 0;
    }
    else if(pid == 0)
    {
        *running = 1;
        for (int i = 0; i < MAX_CLIENTS; ++i)
        {
            if(sockets[i] != connection_descriptor)
            {
                close(sockets[i]);
            }
        }
    }
}

void handle_messages(int message_id)
{
    Text_message text_msg;
    long v;

    v = msgrcv(message_id, &text_msg, sizeof(text_msg), 0, IPC_NOWAIT);
    if(v < 0)
    {
        return;
    }
    if(text_msg.mtype == 2)
    {
        close(text_msg.descriptor);
        for (int i = 0; i < MAX_CLIENTS; ++i)
        {
            if(sockets[i] == text_msg.descriptor)
            {
                sockets[i] = 0;
            }
        }
    }
    else {
        char key[256];
        extract_key(text_msg.mtext, key);


        for (int i = 0; i < MAX_STORE_SIZE; ++i)
        {
            if (strcmp(shared_memory_subs[i].key, key) == 0)
            {
                for (int j = 0; j < MAX_CLIENTS; ++j)
                {
                    char message[256];
                    //cut_garbage(text_msg.mtext, &message);
                    if(shared_memory_subs[i].subs[j] != 0)
                    {
                        printf("sub-descriptor -> %d\n", shared_memory_subs[i].subs[j]);
                        write(shared_memory_subs[i].subs[j], text_msg.mtext, sizeof(text_msg.mtext));
                    }
                }
            }
        }
        if(text_msg.mtype == 3)
        {
            del_sub(key);
        }

        printf("key -> %s\n", key);

        if (v < 0) {
            printf("error reading from queue\n");
        } else {
            printf("[%ld] %s\n", text_msg.mtype, text_msg.mtext);
        }
    }

}

void handleUserInput(struct Statement *statement, int connection_descriptor, int* isRunningTransaction, int* shared_mem, int* quit, int msg_id)
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
    else if(statement->keyExists && statement->valueExists && (strcmp(statement->command, "PUT") == 0 || strcmp(statement->command, "put") == 0))
    {
        char msg[1024] = "PUT:";
        strcat(msg, statement->key);
        strcat(msg, ":");
        strcat(msg, statement->value);
        strcat(msg, "\n\0");
        put(statement->key, statement->value, msg, sizeof(msg));
        Text_message text_msg;
        strcpy(text_msg.mtext, msg);
        text_msg.mtype = 1;
        text_msg.descriptor = connection_descriptor;
        msgsnd(msg_id, &text_msg, sizeof (text_msg), 0);
        write(connection_descriptor, msg, sizeof(msg));
    }
    else if(statement->keyExists && (strcmp(statement->command, "GET") == 0 || strcmp(statement->command, "get") == 0))
    {
        int res;
        char result[256];
        get(statement->key, result);
        char msg[1024] = "GET:";
        strcat(msg, statement->key);
        strcat(msg, ":");
        strcat(msg, result);
        strcat(msg, "\n");
        write(connection_descriptor, msg, sizeof(msg));
    }
    else if(statement->keyExists && (strcmp(statement->command, "DEL") == 0 || strcmp(statement->command, "del") == 0))
    {
        int res;
        res = del(statement->key);
        char msg[1024] = "DEL:";
        if(res == 1) {
            strcat(msg, statement->key);
            strcat(msg, ":key_deleted\n");
        }
        else {
            strcat(msg, statement->key);
            strcat(msg, ":key_nonexistent\n");
        }
        Text_message text_msg;
        strcpy(text_msg.mtext, msg);
        text_msg.mtype = 3;
        text_msg.descriptor = connection_descriptor;
        msgsnd(msg_id, &text_msg, sizeof (text_msg), 0);
        write(connection_descriptor, msg, sizeof(msg));
    }
    else if(strcmp(statement->command, "QUIT") == 0 || strcmp(statement->command, "quit") == 0)
    {
        if(*isRunningTransaction)
        {
            *shared_mem = 0;
            char msg[] = "TRANSACTION ENDED\n";
            write(connection_descriptor, msg, sizeof(msg));
        }

        for (int i = 0; i < MAX_STORE_SIZE; ++i)
        {
            for (int j = 0; j < MAX_STORE_SIZE; ++j)
            {
                if(shared_memory_subs[i].subs[j] == connection_descriptor)
                {
                    shared_memory_subs[i].subs[j] = 0;
                }
            }
        }

        Text_message text_msg;
        text_msg.mtype = 2;
        text_msg.descriptor = connection_descriptor;
        msgsnd(msg_id, &text_msg, sizeof (text_msg), 0);

        *quit = 0;
        close(connection_descriptor);
        exit(0);
    }
    else if(strcmp(statement->command, "SUB") == 0 || strcmp(statement->command, "sub") == 0)
    {
        sub(statement->key, connection_descriptor);
    }
    else
    {
        char msg[] = "There is no such command like this\n";
        write(connection_descriptor, msg, sizeof(msg));
    }
}