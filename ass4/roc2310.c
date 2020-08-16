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

#define BUFFER_SIZE 79
#define BASE 10
#define MINIM_ARGS 3

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

/** 
 * Output error message for status and return status
 *	- Returns nothing
 *	- Prints exit status messages out to stderr(Standard error) 
 * @param status: output status
 */
Status exit_message(Status status) {
    const char* messages[] = {"", //0
            "Usage: roc2310 id mapper {airports}\n", //1
            "Invalid mapper port\n", //2
            "Mapper required\n", //3
            "Failed to connect to mapper\n", //4
            "No map entry for destination\n", //5
            "Failed to connect to at least one destination\n"}; //6
    fputs(messages[status], stderr);
    return status;
}

/**
 * @brief  a helper function to connect all the airport port
 * @note   if has mapper else no mapper
 * may raise mapper needed error, then exit with code 3
 * @param  hasMapper: true if has mapper
 * @param  numberOfAirport: number of airport from argument
 * @param  failed: true if error during connection
 * @param  planeId: plane name
 * @param  portNumberString: airport port number string version 
 * @param  argv: run arguments
 * @param  portNumber: airport port number
 * @retval boolean value: failed, if failed during connection then true
 */
bool connect_port(bool hasMapper, int numberOfAirport, bool failed, 
        const char* planeId, const char* portNumberString[], 
        const char* argv[], double portNumber[]) {
    if (hasMapper) {
        for (int i = 0; i < numberOfAirport; i++) {
            // add plane to airport and print airport info
            int fileDescriptor = connect_to_port(portNumberString[i]);
            if (fileDescriptor == -1) {
                failed = true;
                break;
            }
            handle_connection_plane(planeId, fileDescriptor);
        }
    } else {  // no mapper (-)
        for (int i = 0; i < numberOfAirport; i++) {
            // firstly check if we need a mapper
            char* portError;
            portNumber[i] = strtol(argv[MINIM_ARGS + i], &portError, BASE);
            if (*portError != '\0' || portNumber[i] <= 0 
                    || portNumber[i] > MAXMI_VALID_PORT) {
                exit_message(MAPPER_NEEDED); // not a good port
                exit(3);
            } 

            // add plane to airport and print airport info
            int fileDescriptor = connect_to_port(argv[MINIM_ARGS + i]);
            if (fileDescriptor == -1) {
                failed = true;
                break;
            }
            handle_connection_plane(planeId, fileDescriptor);
        }
    }
    return failed;
}

/**
 * @brief  try to connect the mapper
 * use to detect if error in the mapper port
 * @note   may raise unable to connnection error, then exit with code 2
 * @param  argv: run arguments
 * @retval fileDescriptor: if connection successful
 */
int try_connect_mapper(const char* argv[]) {
    if (!strcmp("0", argv[2])) { // special case
        exit_message(INVALID_MAPPER_PORT);
        exit(2);
    }
    char* mappingPortError;
    int mappingPort = strtol(argv[2], &mappingPortError, BASE);
    if (mappingPort != 0) { // strtol return 0 means fault
        if (*mappingPortError != '\0' || mappingPort <= 0 
                || mappingPort > MAXMI_VALID_PORT) {
            exit_message(INVALID_MAPPER_PORT);
            exit(2);
        }
    } else {
        exit_message(INVALID_MAPPER_PORT); // not a number
        exit(2);
    }
    int fileDescriptor = connect_to_port(argv[2]);
    if (fileDescriptor == -1) {
        exit_message(UNABLE_TO_CONNECT_MAPPER);
        exit(4);
    }
    return fileDescriptor;
}

/**
 * @brief  if has mapper connect mapper first, convert all id into port number
 * then try to connect each airport port 
 * @note   if has mapper else no mapper
 * may raise mapper no destination error, then exit with code 5
 * @param  hasMapper: true if has mapper
 * @param  numberOfAirport: number of airport from argument
 * @param  failed: true if error during connection
 * @param  portNumberString: airport port number string version 
 * @param  argv: run arguments
 * @param  portNumber: airport port number
 * @retval boolean value: failed, if failed during connection then true
 */    
bool handle_all_connection(const char* argv[], int numberOfAirport, 
        bool hasMapper, double portNumber[], const char* portNumberString[]) {
    bool failed = false; // test if connect failed at lease once;
    char buffer[numberOfAirport + 1][BUFFER_SIZE];
    const char* planeId = argv[1]; // load id of plane
    // load mapper port (optional)
    if (!strncmp("-", argv[2], 1)) { // Mapper is dash do nothing
    } else {
        int fileDescriptor = try_connect_mapper(argv);
        hasMapper = true;
        int fileDescriptor2 = dup(fileDescriptor);
        FILE* streamWrite = fdopen(fileDescriptor, "w");
        FILE* streamRead = fdopen(fileDescriptor2, "r");

        // conver all to port number
        for (int i = 0; i < numberOfAirport; i++) {   
            char* portError;
            portNumber[i] = strtol(argv[MINIM_ARGS + i], &portError, BASE);
            if (*portError != '\0' || portNumber[i] <= 0 
                    || portNumber[i] > MAXMI_VALID_PORT) {
                failed = true;
            } else {
                portNumberString[i] = argv[MINIM_ARGS + i]; // WORKS FINE
            }

            if (failed) { // kind of second chance
                fprintf(streamWrite, "?%s\n", argv[MINIM_ARGS + i]);
                fflush(streamWrite);
                if (fgets(buffer[i], BUFFER_SIZE, streamRead) != NULL) { 
                    if (!strncmp(";", buffer[i], 1)) { 
                        exit_message(MAPPER_NO_DEST);
                        exit(5);
                    } else {
                        buffer[i][strcspn(buffer[i], "\n")] = '\0';
                        portNumberString[i] = buffer[i]; 
                        failed = false; // reinitialize
                    }
                } else {
                    exit_message(MAPPER_NO_DEST);
                    exit(5);
                }
            }
        }
        fclose(streamRead); // connection terminated
        fclose(streamWrite);
    }
    
    // try to connect all port number
    failed = connect_port(hasMapper, numberOfAirport, failed, planeId, 
            portNumberString, argv, portNumber);
    return failed;
}

int main(int argc, char const* argv[]) {
    if (argc < MINIM_ARGS) {
        return exit_message(WRONG_ARG_NUMBER);
    }
    int numberOfAirport = argc - MINIM_ARGS; // airport starts from argv[4]
    double portNumber[numberOfAirport + 1];
    const char* portNumberString[numberOfAirport + 1];
    bool hasMapper = false;

    // failed means there is error during connection
    bool failed = handle_all_connection(argv, numberOfAirport, hasMapper, 
            portNumber, portNumberString);
    
    if (failed) {
        return exit_message(UNABLE_TO_CONNECT_DEST);
    }
    return exit_message(NORMAL_OPERATION);
}