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

queue_t *globalQueue;
hashmap_t *globalMap;

pthread_mutex_t lock;

int openListenFD(int portNumber, int maxEntries);
void *connectionHandler(void *arg);

void error(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void destroyHashMap() {
}

void destroyQueue() {
}

int main(int argc, char *argv[]) {
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

    //Set arguments to given variables
    struct sockaddr_in clientAddr;
    int connectionFD, clientLength;

    int numWorkers = atoi(argv[1]);
    int portNumber = atoi(argv[2]);
    int maxEntries = atoi(argv[3]);

    globalQueue = create_queue(); //Create a new queue
    globalMap = create_map(maxEntries, jenkins_one_at_a_time_hash, destroyHashMap); //Create a new hashmap

    pthread_mutex_init(&lock, NULL);

    pthread_t threadIDs[numWorkers]; //Spawn NUM_WORKERS amount of threads
    for (int i = 0; i < numWorkers; i++) {
        pthread_create(&threadIDs[i], NULL, connectionHandler, &connectionFD);
    }

    int listenFD = openListenFD(portNumber, maxEntries); //Open socket, bind to specified PORT_NUMBER and infinitely listen
    clientLength = sizeof(clientAddr);

    //int i = 0;

    //request_header_t requestHeader;

    //Accept a client's connection
    if ((connectionFD = accept(listenFD, (struct sockaddr*) &clientAddr, (socklen_t*) &clientLength)) < 0)
        error("ERROR on accept");

    enqueue(globalQueue, &connectionFD);

//    read(connectionFD, &requestHeader, sizeof(requestHeader));

//    printf("%d\n", requestHeader.request_code);

    //printf("%d\n", *(int*)globalQueue->front->item);



    /*while (connectionFD = accept(listenFD, (struct sockaddr*) &clientAddr, (socklen_t*) &clientLength)) {
        if (pthread_create(&threadIDs[i], NULL, connectionHandler, connectionFD) < 0)
            error("ERROR on creating thread");

        pthread_join(threadIDs[i], NULL);
    }

    if (connectionFD < 0)
        error("ERROR on accept");
    */



    exit(0);
}

void *connectionHandler(void *arg) {
    //pthread_mutex_lock(&lock);
    //int clientFD = (int*) arg;
    //pthread_mutex_unlock(&lock);
    int clientFD = *(int*) arg;
    request_header_t requestHeader;
    read(clientFD, &requestHeader, sizeof(requestHeader));
    printf("%d\n", requestHeader.request_code);
    return NULL;
}

int openListenFD(int portNumber, int maxEntries) {
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
    listen(sockFD, maxEntries);

    return sockFD;
}
