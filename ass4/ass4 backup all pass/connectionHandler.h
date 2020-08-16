#ifndef CONNECTION_HANDLER_H_
#define CONNECTION_HANDLER_H_
#include <stdbool.h>
#include "shared.h"

void handle_connection(Mapper* mapping, int connectionFd);

void handle_connection_airport(Airport* airport, int connectionFd);

void handle_connection_plane(const char* planeId, int connectionFd);

int connect_to_port(const char* port);

#endif