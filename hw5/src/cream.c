#include "cream.h"
#include "utils.h"
#include "queue.h"
#include "hashmap.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
//#include "debug.h"

queue_t *globalQueue;
hashmap_t *globalMap;

int listenFD, connectionFD;

int openListenFD(int portNumber);
void *connectionHandler(void *arg);

void sigint_handler(int sig) {
    shutdown(listenFD, 2);
    printf("\n");
    exit(EXIT_SUCCESS);
}

void error(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void destroyHashMap(map_key_t key, map_val_t val) {
    free(key.key_base);
    free(val.val_base);
}

int main(int argc, char *argv[]) {
    signal(SIGINT, sigint_handler);
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            printf("./cream [-h] NUM_WORKERS PORT_NUMBER MAX_ENTRIES\n\
-h                 Displays this help menu and returns EXIT_SUCCESS.\n\
NUM_WORKERS        The number of worker threads used to service requests.\n\
PORT_NUMBER        Port number to listen on for incoming connections.\n\
MAX_ENTRIES        The maximum number of entries that can be stored in `cream`'s underlying data store.\n");
            exit(EXIT_SUCCESS);
        }
    }
    if (argc < 4 || argc > 4) {
        printf("Not enough arguments given\n");
        exit(EXIT_FAILURE);
    }

    //Set arguments to given variables
    struct sockaddr_in clientAddr;
    int clientLength;

    int numWorkers = atoi(argv[1]);
    int portNumber = atoi(argv[2]);
    int maxEntries = atoi(argv[3]);

    globalQueue = create_queue(); //Create a new queue
    globalMap = create_map(maxEntries, jenkins_one_at_a_time_hash, destroyHashMap); //Create a new hashmap

    pthread_t threadIDs[numWorkers]; //Spawn NUM_WORKERS amount of threads
    for (int i = 0; i < numWorkers; i++) {
        pthread_create(&threadIDs[i], NULL, connectionHandler, NULL);
    }

    listenFD = openListenFD(portNumber); //Open socket, bind to specified PORT_NUMBER and infinitely listen
    clientLength = sizeof(clientAddr);

    //Accept a client's connection
    while ((connectionFD = accept(listenFD, (struct sockaddr*) &clientAddr, (socklen_t*) &clientLength))) {
        if (connectionFD < 0)
            error("ERROR on accept");
        enqueue(globalQueue, &connectionFD);
    }

    exit(EXIT_SUCCESS);
}

void *connectionHandler(void *arg) {
    while (1) {
        int clientFD = *(int*)dequeue(globalQueue);
        pthread_detach(pthread_self());
        request_header_t requestHeader;
        response_header_t responseHeader;
        read(clientFD, &requestHeader, sizeof(requestHeader));
        if (requestHeader.request_code == PUT) {
            if (requestHeader.key_size < MIN_KEY_SIZE || requestHeader.key_size > MAX_KEY_SIZE || requestHeader.value_size < MIN_VALUE_SIZE || requestHeader.value_size > MAX_VALUE_SIZE) {
                responseHeader.response_code = BAD_REQUEST;
                responseHeader.value_size = 0;
                write(clientFD, &responseHeader, sizeof(responseHeader));
                close(connectionFD);
            } else {
                void *keyPtr = malloc(requestHeader.key_size);
                void *valPtr = malloc(requestHeader.value_size);
                //*keyPtr = requestHeader;
                read(clientFD, keyPtr, requestHeader.key_size);
                //*valPtr = requestHeader + requestHeader.key_size;
                read(clientFD, valPtr, requestHeader.value_size);
                map_key_t key = {keyPtr, requestHeader.key_size};
                map_val_t val = {valPtr, requestHeader.value_size};
                put(globalMap, key, val, true);

                responseHeader.response_code = OK;
                responseHeader.value_size = 0;
                write(clientFD, &responseHeader, sizeof(responseHeader));
                close(connectionFD);
            }
        } else if (requestHeader.request_code == GET) {
            if (requestHeader.key_size < MIN_KEY_SIZE || requestHeader.key_size > MAX_KEY_SIZE) {
                responseHeader.response_code = BAD_REQUEST;
                responseHeader.value_size = 0;
                write(clientFD, &responseHeader, sizeof(responseHeader));
                close(connectionFD);
            } else {
                void *keyPtr = malloc(requestHeader.key_size);
                read(clientFD, keyPtr, requestHeader.key_size);
                map_key_t key = {keyPtr, requestHeader.key_size};
                map_val_t val = get(globalMap, key);
                if (val.val_base != NULL && val.val_len != 0) {
                    responseHeader.response_code = OK;
                    responseHeader.value_size = val.val_len;
                    write(clientFD, &responseHeader, sizeof(responseHeader));
                    write(clientFD, val.val_base, val.val_len);
                    close(connectionFD);
                } else {
                    responseHeader.response_code = NOT_FOUND;
                    responseHeader.value_size = 0;
                    write(clientFD, &responseHeader, sizeof(responseHeader));
                    close(connectionFD);
                }
                free(keyPtr);
            }
        } else if (requestHeader.request_code == EVICT) {
            if (requestHeader.key_size < MIN_KEY_SIZE || requestHeader.key_size > MAX_KEY_SIZE) {
                responseHeader.response_code = BAD_REQUEST;
                responseHeader.value_size = 0;
                write(clientFD, &responseHeader, sizeof(responseHeader));
                close(connectionFD);
            } else {
                void *keyPtr = malloc(requestHeader.key_size);
                read(clientFD, keyPtr, requestHeader.key_size);
                map_key_t key = {keyPtr, requestHeader.key_size};
                delete(globalMap, key);
                responseHeader.response_code = OK;
                responseHeader.value_size = 0;
                write(clientFD, &responseHeader, sizeof(responseHeader));
                close(connectionFD);
            }
        } else if (requestHeader.request_code == CLEAR) {
            clear_map(globalMap);
            responseHeader.response_code = OK;
            responseHeader.value_size = 0;
            write(clientFD, &responseHeader, sizeof(responseHeader));
            close(connectionFD);
        } else {
            responseHeader.response_code = UNSUPPORTED;
            responseHeader.value_size = 0;
        }
    }
    return NULL;
}

int openListenFD(int portNumber) {
    struct sockaddr_in servAddr;

    int sockFD, optval = 1;
    if ((sockFD = socket(AF_INET, SOCK_STREAM, 0)) < 0) //Open a socket
        error("ERROR on opening socket");

    if (setsockopt(sockFD, SOL_SOCKET, SO_REUSEADDR, (const void*) &optval, sizeof(int)) < 0)
        error("ERROR on setsockopt");

    bzero((char*)&servAddr, sizeof(servAddr)); //Set to 0

    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons((unsigned short)portNumber);

    //Bind socket to PORT_NUMBER
    if (bind(sockFD, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0)
        error("ERROR on binding");
    //Infinitely listen?
    listen(sockFD, 1);

    return sockFD;
}
