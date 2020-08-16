#ifndef SHARED_H_
#define SHARED_H_
#include <stdbool.h>
#include <stdint.h>
#include <semaphore.h>
#include <stdio.h>

#define VALID_CHARS 256
#define MAXMI_VALID_PORT 65536

/* a Node in the trie stored in Mapper and Airport */
struct TrieNode {
    long portNumber;
    char namePart;
    struct TrieNode* childNodes[VALID_CHARS];
    int timeVisited; // use for roc2310 time count;
};
typedef struct TrieNode TrieNode;

/* the airport */
typedef struct {
    const char* airportId;
    const char* airportInfo;
    uint16_t port;
    TrieNode* planeRootTrieNode; // the plane have visited this airport
    sem_t* semaphore;
    int maxNameSize; // use for print name (malloc)
    int fileDescriptor; // for connect mapper
} Airport;

/* the local mapper connected airports */
typedef struct {
    uint16_t port;
    TrieNode* mapperRootTrieNode; // trie can print content in lexi order
    sem_t* semaphore;
    int maxNameSize; // use for print name (malloc)
} Mapper;

Mapper* mapping_create();

Airport* airport_create();

void airport_set_plane_id(Airport* airport, const char* planeName);

void mapping_set_port_number(Mapper* mapping, const char* airportName, 
        long portNumber);

long mapping_get_port_number(Mapper* mapping, const char* airportName);

void mapping_print_airport_port_numbers(Mapper* mapping, FILE* streamWrite);

void airport_print_plane(Airport* airport, FILE* streamWrite);

bool is_valid_name(const char* name);

#endif