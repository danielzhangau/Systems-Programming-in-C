#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <netdb.h>
#include <pthread.h>
#include "shared.h"
#include "connectionHandler.h"

#define BUFFER_SIZE 80

/** An enum
 * Define exit status 
 */
typedef enum {
    NORMAL_OPERATION = 0,
    WRONG_ARG_NUMBER = 1,
    INVALID_MAPPER_PORT = 2,
    MAPPER_NEEDED = 3,
    UNABLE_TO_CONNECT_MAPPER = 4,
    MAPPER_NO_DEST = 5,
    UNABLE_TO_CONNECT_DEST = 6
} Status;

/** Output error message for status and return status
 *	- Returns nothing
 *	- Prints exit status messages out to stderr(Standard error) 
 */
Status exit_message(Status s) {
    const char* messages[] = {"", //0
            "Usage: roc2310 id mapper {airports}\n", //1
            "Invalid mapper port\n", //2
            "Mapper required\n", //3
            "Failed to connect to mapper\n", //4
            "No map entry for destination\n", //5
            "Failed to connect to at least one destination\n"}; //6
    fputs(messages[s], stderr);
    return s;
}

int main(int argc, char const* argv[]) {
    if (argc < 3) {
        return exit_message(WRONG_ARG_NUMBER);
    }
    int numberOfPlane = argc - 3;
    double portNum[numberOfPlane + 1];
    const char* portNumString[numberOfPlane + 1];
    char buffer[numberOfPlane + 1][BUFFER_SIZE];
    bool failed = false; // test if connect failed at lease once;
    bool hasMapper = false;

    // load id of plane
    const char* planeId = argv[1];
    // load mapper port (optional)
    if (!strncmp("-", argv[2], 1)) { // Mapper is dash
        // do nothing
    } else {
        if (!strcmp("0", argv[2])) { // special case
            return exit_message(INVALID_MAPPER_PORT);
        }
        char* mappingPortError;
        int mappingPort = strtol(argv[2], &mappingPortError, 10);
        if (mappingPort != 0) { // strtol return 0 means fault
            if (*mappingPortError != '\0' || mappingPort <= 0 
                    || mappingPort > 65535) {
                return exit_message(INVALID_MAPPER_PORT);
            }
        } else {
            return exit_message(INVALID_MAPPER_PORT); // not a number
        }
        int fileDescriptor = connect_to_port(argv[2]);
        if (fileDescriptor == -1) {
            return exit_message(UNABLE_TO_CONNECT_MAPPER);
        }

        hasMapper = true;

        int fileDescriptor2 = dup(fileDescriptor);
        FILE* streamWrite = fdopen(fileDescriptor, "w");
        FILE* streamRead = fdopen(fileDescriptor2, "r");

        // conver all to port number
        for (int i = 0; i < numberOfPlane; i++) {   
            char* portError;
            portNum[i] = strtol(argv[3 + i], &portError, 10);
            if (*portError != '\0' || portNum[i] <= 0 || portNum[i] > 65535) {
                failed = true;
            } else {
                portNumString[i] = argv[3 + i]; // WORKS FINE
            }
            if (failed) { // kind of second chance
                fprintf(streamWrite, "?%s\n", argv[3 + i]);
                fflush(streamWrite);
                if (fgets(buffer[i], BUFFER_SIZE, streamRead) != NULL) { 
                    if (!strncmp(";", buffer[i], 1)) { 
                        return exit_message(MAPPER_NO_DEST);
                    } else {
                        buffer[i][strcspn(buffer[i], "\n")] = '\0';
                        portNumString[i] = buffer[i]; 
                        failed = false; // reinitialize
                    }
                } else {
                    return exit_message(MAPPER_NO_DEST);
                }
            }
        }
        // connection terminated
        fclose(streamRead);
        fclose(streamWrite);
    }
    
    if (hasMapper) {
        for (int i = 0; i < numberOfPlane; i++) {
            // add plane to airport and print airport info
            int fileDescriptor = connect_to_port(portNumString[i]);
            if (fileDescriptor == -1) {
                failed = true;
                break;
            }
            handle_connection_plane(planeId, fileDescriptor);
        }
    } else {  // no mapper (-)
        for (int i = 0; i < numberOfPlane; i++) {
            // firstly check if we need a mapper
            char* portError;
            portNum[i] = strtol(argv[3 + i], &portError, 10);
            if (*portError != '\0' || portNum[i] <= 0 || portNum[i] > 65535) {
                return exit_message(MAPPER_NEEDED); // not a good port
            } 

            // add plane to airport and print airport info
            int fileDescriptor = connect_to_port(argv[3 + i]);
            if (fileDescriptor == -1) {
                failed = true;
                break;
            }
            handle_connection_plane(planeId, fileDescriptor);
        }
    }
    
    if (failed) {
        return exit_message(UNABLE_TO_CONNECT_DEST);
    }

    return exit_message(NORMAL_OPERATION);
}