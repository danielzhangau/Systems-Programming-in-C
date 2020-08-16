#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <netdb.h>
#include "shared.h"

#define BUFFER_SIZE 150 

// Used to pass arguments to process_thread()
typedef struct {
    Mapper* mapping;  // for mapper2310
    int connectionFd;
    Airport* airport; // for control2310
    bool decide; // true run mapper, false run control
    const char* planeId;
} ProcessThreadArgs;

// Used to return arguments from a parsed message
typedef struct {
    bool valid; 
    char* idName;
    unsigned long portNumber;
} MessageInfo;

/**
 * @brief  (MAPPER) parses and actions a ask message ?ID
 * @param  mapping: the local map 
 * @param  message: pointer to first character of deliver message arguments
 * @param  streamWrite: place to print
 * @retval None
 */
void parse_ask_message(Mapper* mapping, char* message, FILE* streamWrite) {
    MessageInfo info;
    info.valid = true;

    // cut message to parse portion without invalid chars
    message[strcspn(message, "\r")] = '\0';
    message[strcspn(message, "\n")] = '\0';

    // get id(name) and validate
    info.idName = message;
    if (!is_valid_name(info.idName)) {
        info.valid = false;
    }

    if (!info.valid) {
        return;
    }
    int portNum = mapping_get_port_num(mapping, info.idName);
    if (portNum != 0) {
        fprintf(streamWrite, "%d\n", portNum);
        fflush(streamWrite);
    } else {
        fprintf(streamWrite, ";\n");
        fflush(streamWrite);
    }
}

/**
 * @brief  (MAPPER) parses and actions a add message: !ID:PORT
 * @param  mapping: the local map 
 * @param  message: pointer to first character of deliver message arguments
 * @retval None
 */
void parse_add_message(Mapper* mapping, char* message) {
    MessageInfo info;
    info.valid = true;

    // replace the invalid char with terminate char
    message[strcspn(message, "\r")] = '\0';
    message[strcspn(message, "\n")] = '\0';

    // get id(name) and validate
    info.idName = message;
    int firstColon = strcspn(message, ":");  // use for separate two input
    info.idName[firstColon] = '\0';
    // get port and validate
    char* res = &info.idName[firstColon + 1];
    char* portErr;
    info.portNumber = strtoul(res, &portErr, 10);
    if (info.portNumber < 0 || isspace(res[0])) {
        info.valid = false;
    }

    // validate field1Str
    if (!is_valid_name(info.idName)) {
        info.valid = false;
    }

    if (!info.valid) {
        return;
    }

    mapping_set_port_num(mapping, info.idName, info.portNumber);
}

/**
 * @brief  (MAPPER) parses and actions a all message @
 * @param  mapping: the local map 
 * @param  message: pointer to first char of deliver message arguments
 * @param  streamWrite: place to print
 * @retval None
 */
void parse_all_message(Mapper* mapping, char* message, FILE* streamWrite) {
    if (message[0] != '\n') {
        return;
    }
    
    mapping_print_airport_port_nums(mapping, streamWrite);
}

/**
 * @brief  (AIRPORT) parses and actions a all message log 
 * (plane visited the airport)
 * @param  airport: the local airport 
 * @param  message: pointer to first char of deliver message arguments
 * @param  streamWrite: place to print
 */
void parse_log_message(Airport* airport, char* message, FILE* streamWrite) {
    if (message[0] != '\n') {
        return;
    }

    airport_print_plane(airport, streamWrite);
    fprintf(streamWrite, ".\n");
    fflush(streamWrite);
}

/**
 * @brief  (AIRPORT) similiar to parse_log_message, 
 * but print the airport info asked
 */
void parse_res_message(Airport* airport, char* message, FILE* streamWrite) {
    MessageInfo info;
    info.valid = true;

    // cut message to parse portion without invalid chars
    message[strcspn(message, "\r")] = '\0';
    message[strcspn(message, "\n")] = '\0';

    // get id(name) and validate
    info.idName = message;
    if (!is_valid_name(info.idName)) {
        info.valid = false;
    }

    if (!info.valid) {
        return;
    }

    airport_set_plane_id(airport, info.idName);
    const char* airportInfo = airport->airportInfo;
    if (airportInfo != NULL) {
        fprintf(streamWrite, "%s\n", airportInfo);
        fflush(streamWrite);
    } else {
        return;
    }
}

/**
 * @brief  check if there is unexpected eof on message
 * @param  message: message body
 * @retval true on no, false if yes
 */
bool check_string_eof(char* message) {
    if (message[0] == '\0') {
        return false; // unexpected EOF
    }
    if (message[0] != '\n') {
        return false;
    }
    return true;
}

/**
 * @brief  (MAPPER) Parses all received messages
 * @param  mapping: the local map 
 * @param  streamRead: source for incoming messages
 * @retval None
 */
void parse_messages(Mapper* mapping, FILE* streamRead, FILE* streamWrite) {
    char buffer[BUFFER_SIZE];

    while (fgets(buffer, BUFFER_SIZE, streamRead) != NULL) {
        if (!strncmp("?", buffer, 1)) {
            parse_ask_message(mapping, buffer + 1, streamWrite);
            if (check_string_eof(buffer + 1)) {
                break;
            }
        } else if (!strncmp("!", buffer, 1)) {
            parse_add_message(mapping, buffer + 1);
            if (check_string_eof(buffer + 1)) {
                break;
            }
        } else if (!strncmp("@", buffer, 1)) {
            parse_all_message(mapping, buffer + 1, streamWrite);
        }
    }
}

/**
 * @brief  (AIRPORT) similiar to parse message for mapper
 */
void parse_messages_airport(Airport* airport, FILE* streamRead, 
        FILE* streamWrite) {
    char buffer[BUFFER_SIZE];
    while (fgets(buffer, BUFFER_SIZE, streamRead) != NULL) {
        if (!strncmp("log", buffer, 3)) {
            parse_log_message(airport, buffer + 3, streamWrite); 
            if (check_string_eof(buffer + 3)) {
                break;
            }
        } else {
            parse_res_message(airport, buffer, streamWrite);
        }
    }
}

/**
 * @brief  (ROC) send the plane name to airport for record
 * @param  planeId: plane name
 * @param  streamWrite: place to write
 * @retval None
 */
void send_message_plane(const char* planeId, FILE* streamWrite) {
    fprintf(streamWrite, "%s\n", planeId);
    fflush(streamWrite);
}

/**
 * @brief  (MAPPER or AIRPORT) handles all communication to a connection 
 * made to the command socket
 * @param  passArgs: pointer to ProcessThreadArgs
 * @note   the passArgs->decide true run mapper, otherwise airport
 * @retval None
 */
void* process_thread(void* passArgs) {
    ProcessThreadArgs* args = (ProcessThreadArgs*)passArgs;
    int connectionFd = args->connectionFd;
    bool decide = args->decide;
    if (decide) { // true for mapper
        Mapper* mapping = args->mapping;
        free(args);        

        // create comm streams 
        int connectionFd2 = dup(connectionFd);
        FILE* streamWrite = fdopen(connectionFd, "w");
        FILE* streamRead = fdopen(connectionFd2, "r");

        parse_messages(mapping, streamRead, streamWrite);

        // connection terminated
        fclose(streamRead);
        fclose(streamWrite);
    } else {      // false for control
        Airport* airport = args->airport;
        free(args);

        // create comm streams 
        int connectionFd2 = dup(connectionFd);
        FILE* streamWrite = fdopen(connectionFd, "w");
        FILE* streamRead = fdopen(connectionFd2, "r");

        parse_messages_airport(airport, streamRead, streamWrite);

        // connection terminated
        fclose(streamRead);
        fclose(streamWrite);
    }
    return NULL;
}

/**
 * @brief  (MAPPER) creates a new thread to handle all communication to a 
 * established inbound connection on the command port
 * @param  mapping: local mapper
 * @param  connectionFd: file descriptor for established connection
 * @retval None
 */
void handle_connection(Mapper* mapping, int connectionFd) {
    // create thread to handle connection
    pthread_t tid;
    ProcessThreadArgs* args = (ProcessThreadArgs*)
            malloc(sizeof(ProcessThreadArgs));
    args->connectionFd = connectionFd;
    args->mapping = mapping;
    args->decide = true;
    pthread_create(&tid, NULL, process_thread, args);

    // its resources are auto released back to the system without the need 
    // for another thread to join with the terminated thread.
    pthread_detach(tid);
}

/**
 * @brief (AIRPORT) similiar to handle_connection but for airport
 */
void handle_connection_airport(Airport* airport, int connectionFd) {
    // create thread to handle connection
    pthread_t tid;
    ProcessThreadArgs* args = (ProcessThreadArgs*)
            malloc(sizeof(ProcessThreadArgs));
    args->connectionFd = connectionFd;
    args->airport = airport;
    args->decide = false;
    pthread_create(&tid, NULL, process_thread, args);

    pthread_detach(tid);
}

/**
 * @brief (ROC) similiar to handle_connection but for plane and simpler
 */
void handle_connection_plane(const char* planeId, int connectionFd) {
    int connectionFd2 = dup(connectionFd);
    FILE* streamWrite = fdopen(connectionFd, "w");
    FILE* streamRead = fdopen(connectionFd2, "r");

    send_message_plane(planeId, streamWrite);
    char buffer[BUFFER_SIZE];
    if (fgets(buffer, BUFFER_SIZE, streamRead) != NULL) {
        printf("%s", buffer);
        fflush(stdout);
    }
    

    // connection terminated
    fclose(streamRead);
    fclose(streamWrite);
    return;
}

/**
 * @brief  establish a connect to a specific port for communication
 * @param  port: port number to establish connect
 * @retval file descriptor
 */
int connect_to_port(const char* port) { 
    // get address info
    struct addrinfo* addressInfo = 0;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;       
    hints.ai_socktype = SOCK_STREAM;
    int addressError;
    if (addressError = getaddrinfo("localhost", (const char*)port, &hints, 
            &addressInfo), addressError) {
        freeaddrinfo(addressInfo);
        return -1;
    }

    // connect local port
    int fileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(fileDescriptor, (struct sockaddr*)addressInfo->ai_addr,
            sizeof(struct sockaddr))) {
        return -1;
    }
    return fileDescriptor;
}