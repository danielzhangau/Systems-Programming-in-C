#ifndef CONNECTION_HANDLER_H_
#define CONNECTION_HANDLER_H_
#include <stdbool.h>
#include "shared.h"

void handle_connection(Mapper* mapping, int connectionFD);

void handle_connection_airport(Airport* airport, int connectionFD);

void handle_connection_plane(const char* planeId, int connectionFD);

int connect_to_port(const char* port);

#endif