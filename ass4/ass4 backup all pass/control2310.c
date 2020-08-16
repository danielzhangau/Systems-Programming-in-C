#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <netdb.h>
#include <pthread.h>
#include <string.h>
#include "shared.h"
#include "connectionHandler.h"

#define BUFFER_SIZE 79
#define LISTEN 15

/** An enum
 * Define exit status 
 */
typedef enum {
    NORMAL_OPERATION = 0,
    WRONG_ARG_NUMBER = 1,
    INVALID_CHAR = 2,
    INVALID_PORT = 3,
    UNABLE_TO_CONNECT = 4
} Status;

/** Output error message for status and return status
 *	- Returns nothing
 *	- Prints exit status messages out to stderr(Standard error) 
 */
Status exit_message(Status s) {
    const char* messages[] = {"", //0
            "Usage: control2310 id info [mapper]\n", //1
            "Invalid char in parameter\n", //2
            "Invalid port\n", //3
            "Can not connect to map\n"}; //4
    fputs(messages[s], stderr);
    return s;
}

/**
 * @brief  a pop up function use specially to send mapper message 
 * @note   only run if has mapper: !..:..
 * @param  airport: a reference to the airport  
 */
void load_mapper_infor(Airport* airport) {
    FILE* streamWrite = fdopen(airport->fileDescriptor, "w");
    fprintf(streamWrite, "!%s:%d\n", airport->airportId, airport->port);
    fflush(streamWrite);
    // connection terminated
    fclose(streamWrite);
}

/**
 * @brief  Binds to an unspecified port 
 * prints the port number 
 * and spawns new threads to deal every incomming connection
 * @note   if any error occurs (listen or bind), code exits with 5.
 * must return a void* and take a void* argument
 * @param  passArg: a reference to the local map  
 */
void* bind_and_listen(void* passArg) {
    Airport* airport = (Airport*)passArg;
    
    struct addrinfo* addressInfo = NULL; // get local address info
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;  
    int addressError;
    if (addressError = getaddrinfo("localhost", 0, &hints, &addressInfo), 
            addressError) { 
        freeaddrinfo(addressInfo);
        exit(5);
    }
    
    // create a socket and bind it to an unspecified port
    int localSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (bind(localSocket, (struct sockaddr*)addressInfo->ai_addr, 
            sizeof(struct sockaddr))) {
        exit(5);
    }

    // print command port
    struct sockaddr_in serverAddre;
    memset(&serverAddre, 0, sizeof(struct sockaddr_in));
    socklen_t addrLen = sizeof(struct sockaddr_in);
    if (getsockname(localSocket, (struct sockaddr*)&serverAddre, &addrLen)) {
        exit(5);
    }
    uint16_t port = ntohs(serverAddre.sin_port);
    printf("%u\n", port);
    fflush(stdout);
    airport->port = port;
    if (airport->fileDescriptor != 0) {
        load_mapper_infor(airport);
    }
    
    // listen on the socket for incoming connections
    int connectionFildes;
    if (listen(localSocket, LISTEN)) {    
        exit(5);
    }

    // spawn a thread to handle each incomming connection
    while (connectionFildes = accept(localSocket, 0, 0), 
            connectionFildes >= 0) { 
        handle_connection_airport(airport, connectionFildes);
    }
    return NULL;
}

int main(int argc, char const* argv[]) {
    if (argc < 3 || argc > 4) {  // mapper is optional
        return exit_message(WRONG_ARG_NUMBER);
    }

    Airport* airport = airport_create();
    // load idname of airport
    airport->airportId = argv[1];
    // initialize to indentify either run load_mapper_infor or not
    airport->fileDescriptor = 0; 
    // load info of airport
    airport->airportInfo = argv[2];
    if (!is_valid_name(airport->airportId) 
            || !is_valid_name(airport->airportInfo)) {
        return exit_message(INVALID_CHAR);
    }

    // load Mapper (optional)
    if (argc == 4) {   
        char* mappingPortError;
        int mappingPort = strtol(argv[3], &mappingPortError, 10);
        if (*mappingPortError != '\0' || mappingPort <= 0 
                || mappingPort > 65535) {
            return exit_message(INVALID_PORT);
        }
        airport->fileDescriptor = connect_to_port(argv[3]);
        if (airport->fileDescriptor == -1) {
            return exit_message(UNABLE_TO_CONNECT);
        }
    }
    
    // ignoring/blocking SIGHUP & SIGPIPE signal in multi-threaded program
    sigset_t set; 
    sigemptyset(&set);
    sigaddset(&set, SIGHUP);
    sigaddset(&set, SIGPIPE);
    pthread_sigmask(SIG_BLOCK, &set, NULL);    

    // create connection handling thread
    pthread_t tid;
    pthread_create(&tid, NULL, bind_and_listen, airport); 

    // wait till EOF
    char buffer[BUFFER_SIZE];
    while (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
        // do nothing
    }
    // exit(0);
    pthread_exit(NULL);
}