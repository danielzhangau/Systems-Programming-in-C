#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <netdb.h>
#include <pthread.h>
#include "shared.h"
#include "connectionHandler.h"

#define BUFFER_SIZE 79 // as spec4.1 said max length
#define LISTEN 15 // max 15 hold thread

/**
 * @brief  Binds to an unspecified port 
 * prints the port number 
 * and spawns new threads to deal every incomming connection
 * @note   if any error occurs (listen or bind), code exits with 1.
 * must return a void* and take a void* argument
 * @param  passArg: a reference to the local map  
 */
void* bind_and_listen(void* passArg) {
    Mapper* mapping = (Mapper*)passArg;

    struct addrinfo* addressInfo = NULL; // get local address info
    struct addrinfo hints;
    // setting the whole structure to zero and 
    // then setting the only three fields that need to be set.
    memset(&hints, 0, sizeof(struct addrinfo)); 
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM; // we want TCP
    hints.ai_flags = AI_PASSIVE;  
    int addressError;
    if (addressError = getaddrinfo("localhost", 0, &hints, &addressInfo), 
            addressError) { 
        freeaddrinfo(addressInfo);
        exit(1);
    }
    
    // create a socket and bind it to an unspecified port
    int localSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (bind(localSocket, (struct sockaddr*)addressInfo->ai_addr, 
            sizeof(struct sockaddr))) {
        exit(1);
    }

    // print command port to stdout
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(struct sockaddr_in));
    socklen_t addrLength = sizeof(struct sockaddr_in);
    if (getsockname(localSocket, (struct sockaddr*)&serverAddr, 
            &addrLength)) {
        exit(1);
    }
    uint16_t port = ntohs(serverAddr.sin_port);
    printf("%u\n", port);
    fflush(stdout);
    mapping->port = port;
    
    // listen on the socket for incoming connections
    if (listen(localSocket, LISTEN)) {    
        exit(1);
    }

    // spawn thread to handle new incoming connection
    int connectionFD;
    while (connectionFD = accept(localSocket, 0, 0), 
            connectionFD >= 0) { 
        handle_connection(mapping, connectionFD);
    }
    return NULL;
} 

int main() {
    // create Mapper
    Mapper* mapping = mapping_create();

    // ignoring/blocking SIGHUP & SIGPIPE signal in multi-threaded program
    sigset_t set; 
    sigemptyset(&set);
    sigaddset(&set, SIGHUP);
    sigaddset(&set, SIGPIPE);
    pthread_sigmask(SIG_BLOCK, &set, NULL); // NULL indentical to 0

    // create connection handling thread
    pthread_t tid;
    pthread_create(&tid, NULL, bind_and_listen, mapping); 
    // tid: pthread_create will fill out with infor on the thread it creates

    // wait till EOF
    char buffer[BUFFER_SIZE];
    while (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
        // do nothing
    }
    // exit(0);
    // pthread_join(tid, NULL); // do i need this?
    pthread_exit(NULL);
}