#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <semaphore.h>
#include <string.h>
#include "shared.h"

/** 
 * A non-zero value means the semaphore is shared between processes 
 * and a value of zero means it is shared between threads.
 */
#define SEMA_SHARE_THREAD 0

/**
 * @brief  creates and initalizes an empty mapping
 * @retval a newly created mapper struct
 */
Mapper* mapping_create() {
    Mapper* mapping = (Mapper*)malloc(sizeof(Mapper));
    mapping->maxNameSize = 0;

    // create and init semaphore
    mapping->semaphore = (sem_t*)malloc(sizeof(sem_t));
    sem_init(mapping->semaphore, SEMA_SHARE_THREAD, 1); 

    // create and memset root trie node
    mapping->mapperRootTrieNode = (TrieNode*)malloc(sizeof(TrieNode));
    memset(mapping->mapperRootTrieNode->childNodes, 0,
            sizeof(TrieNode*) * VALID_CHARS);    

    return mapping;
}

/**
 * @brief  creates and initalizes an empty airport
 * @retval newly created airport struct
 */
Airport* airport_create() {
    Airport* airport = (Airport*)malloc(sizeof(Airport));
    airport->maxNameSize = 0;

    // create and init semaphore
    airport->semaphore = (sem_t*)malloc(sizeof(sem_t));
    sem_init(airport->semaphore, SEMA_SHARE_THREAD, 1);
    
    // create and memset root trie node
    airport->planeRootTrieNode = (TrieNode*)malloc(sizeof(TrieNode));
    memset(airport->planeRootTrieNode->childNodes, 0,
            sizeof(TrieNode*) * VALID_CHARS);    

    return airport;
}

/**
 * @brief  finds the given airport within the trie tree
 * if the given name does not exist builds out the trie tree 
 * @param  mapping: the mapping to update
 * @param  airportName: the name of the target airport to find
 * @retval the target airports leaf trie node
 */
TrieNode* mapping_find_trie(Mapper* mapping, const char* airportName) {
    TrieNode* checkNode = mapping->mapperRootTrieNode;
    int nameSize = 0;

    while (airportName[0] != '\0') {
        char next = airportName[0]; // next char in the name
        nameSize++;
        // the node for airport does not exist then construct it
        if (checkNode->childNodes[(unsigned char)next] == NULL) {
            TrieNode* newNode = (TrieNode*)malloc(sizeof(TrieNode));
            newNode->namePart = next;
            newNode->portNumber = 0;
            memset(newNode->childNodes, 0, sizeof(TrieNode*) * VALID_CHARS);
            checkNode->childNodes[(unsigned char)next] = newNode;
        }
        checkNode = checkNode->childNodes[(unsigned char)next];
        airportName += 1; // move on
    }

    if (nameSize > mapping->maxNameSize) {
        mapping->maxNameSize = nameSize;
    }

    return checkNode;
}

/**
 * @brief  finds the given plane within the trie tree
 * if the given name does not exist builds out the trie tree 
 * @note   use for print visted plane in lexicographic order
 * @param  airport: the airport to update
 * @param  planeName: the name of the target plane to find
 * @retval the target planes leaf trie node
 */
TrieNode* airport_find_trie(Airport* airport, const char* planeName) {
    TrieNode* checkNode = airport->planeRootTrieNode;
    int nameSize = 0;

    while (planeName[0] != '\0') {
        char next = planeName[0];
        if (checkNode->childNodes[(unsigned char)next] == NULL) {
            TrieNode* newNode = (TrieNode*)malloc(sizeof(TrieNode));
            newNode->namePart = next;
            newNode->portNumber = 0;
            newNode->timeVisited = 0; // fpr later print
            memset(newNode->childNodes, 0, sizeof(TrieNode*) * VALID_CHARS);
            checkNode->childNodes[(unsigned char)next] = newNode;
        }

        checkNode = checkNode->childNodes[(unsigned char)next];
        planeName += 1;
        nameSize++;
    }

    if (nameSize > airport->maxNameSize) {
        airport->maxNameSize = nameSize;
    }

    return checkNode;
}

/**
 * @brief  record the visted of the plane name to airport
 * @param  airport: the airport to update
 * @param  planeName: the name of the plane update
 * @retval None
 */
void airport_set_plane_id(Airport* airport, const char* planeName) {
    sem_wait(airport->semaphore); // wait state
    TrieNode* node = airport_find_trie(airport, planeName);
    node->portNumber = 1; // use for print recursively, no meaning
    node->timeVisited += 1; 
    sem_post(airport->semaphore); // signal
}

/**
 * @brief  sets the id of the desired airport
 * @param  mapping: the mapping to update
 * @param  airportName: the name of the airport update
 * @param  portNumber: the portNumber the target airport should be set to
 * @retval None
 */
void mapping_set_port_number(Mapper* mapping, const char* airportName, 
        long portNumber) {
    sem_wait(mapping->semaphore);
    TrieNode* node = mapping_find_trie(mapping, airportName);
    if (node->portNumber == 0) {
        node->portNumber = portNumber;
    }
    sem_post(mapping->semaphore);
}

/**
 * @brief  gets the portNumber of the airport from mapper
 * @param  mapping: the Mapper to find
 * @param  airportName: the name of the airport search for
 * @retval the portNumber of airport the desired airport in the local map
 */
long mapping_get_port_number(Mapper* mapping, const char* airportName) {
    sem_wait(mapping->semaphore);
    TrieNode* node = mapping_find_trie(mapping, airportName);
    long returnValue = node->portNumber;
    sem_post(mapping->semaphore);
    return returnValue;
}

/**
 * @brief  the recursive helper function for mapping_print_airport_port_Number
 * @param  node: The trie node to inspect
 * @param  nameStart: pointer to the start of the constructed trie string 
 * @param  nameEnd: pointer to the last char of the constructed trie string
 * @param  streamWrite: place to write
 * @retval None
 */
void mapping_print_name_recursive(TrieNode* node, char* nameStart, 
        char* nameEnd, FILE* streamWrite) {
    for (unsigned char i = 0; i < VALID_CHARS - 1; i++) {
        TrieNode* branch = node->childNodes[i];
        if (branch != NULL) {
            nameEnd[0] = (char)i; 
            nameEnd[1] = '\0';
            // don not print any unnessesary name
            if (branch->portNumber != 0) {
                fprintf(streamWrite, "%s:%ld\n", nameStart, 
                        branch->portNumber);
                fflush(streamWrite);
            }
            // recursive here 
            mapping_print_name_recursive(branch, nameStart, 
                    nameEnd + 1, streamWrite);
        }
    }
}

/**
 * @brief  prints each airport in lexicographic order along with its 
 * portNumber stored in the map delemited by a new line    
 * @note   airports with a portNumber of 0 are not printed
 * @param  mapping: the mapping to check
 * @param  streamWrite: place to write
 * @retval None
 */
void mapping_print_airport_port_numbers(Mapper* mapping, FILE* streamWrite) {
    sem_wait(mapping->semaphore);

    // create a temporary char* which will store the name of each airport 
    // in the trie tree as it is traversed 
    char* name = (char*)malloc(sizeof(char) * (mapping->maxNameSize + 1));
    name[0] = '\0';

    mapping_print_name_recursive(mapping->mapperRootTrieNode, 
            name, name, streamWrite);
    
    free(name);
    sem_post(mapping->semaphore);
}

/**
 * @brief  the recursive helper function for airport_print_plane. 
 * @param  node: The trie node to inspect
 * @param  nameStart: pointer to the start of the constructed trie string 
 * @param  nameEnd: pointer to the last char of the constructed trie string
 * @param  streamWrite: place to write
 * @retval None
 */
void airport_print_name_recursive(TrieNode* node, char* nameStart, 
        char* nameEnd, FILE* streamWrite) {
    for (unsigned char i = 0; i < VALID_CHARS - 1; i++) {
        TrieNode* branch = node->childNodes[i];
        if (branch != NULL) {
            nameEnd[0] = (char)i; 
            nameEnd[1] = '\0';
            if (branch->portNumber != 0) {
                for (int i = 0; i < branch->timeVisited; i++) {
                    fprintf(streamWrite, "%s\n", nameStart);
                    fflush(streamWrite);
                }
            }
            airport_print_name_recursive(branch, nameStart, 
                    nameEnd + 1, streamWrite);
        }
    }
}

/**
 * @brief  prints each plane visited the airport in lexicographic order  
 * delemited by a new line    
 * @param  airport: the airport to check
 * @param  streamWrite: place to write
 * @retval None
 */
void airport_print_plane(Airport* airport, FILE* streamWrite) {
    sem_wait(airport->semaphore);
    char* name = (char*)malloc(sizeof(char) * (airport->maxNameSize + 1));
    name[0] = '\0';

    airport_print_name_recursive(airport->planeRootTrieNode, 
            name, name, streamWrite);
    
    free(name); // cuz malloc
    sem_post(airport->semaphore);
}

/**
 * @brief  checks whether the provided name is valid
 * @note   valid name can't have: '\n', '\r' or ':' & can not be empty
 * @param  name: the name to check
 * @retval true if valid, otherwise false
 */
bool is_valid_name(const char* name) {
    if (name[0] == '\0') {
        return false; // empty name
    }
    while (name[0] != '\0') {
        char checkNamePart = name[0];
        if (checkNamePart == '\n' || checkNamePart == '\r' 
                || checkNamePart == ':') { 
            return false;  // contains invalid namePart
        }
        name++;
    }
    return true;
}