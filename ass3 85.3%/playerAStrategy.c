#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "player.h"
#include "shared.h"

/**
 * @brief  start from currentNode to find the next barrier
 * @param  currentNode: the node start
 * @retval the barrier index
 */
int find_next_barrier(Game* game, PathListNode* currentNode) {
    int barrierIndex;
    int currentIndex = currentNode->site->siteIndex;
    PathListNode* nextNode = currentNode->after;
    for (int i = currentIndex; i < game->pathList->size; i++) {
        if (strcmp(nextNode->site->type, "::") == 0) {
            barrierIndex = nextNode->site->siteIndex;
            break;
        } else {
            nextNode = nextNode->after;
        }   
    }
    return barrierIndex; 
}

/**
 * @brief  start from firstnode in pathlist to find the first non-NUll
 *      node as current node
 * @retval our current location node
 */
PathListNode* find_current_index(Game* game) {
    PathListNode* currentNode = game->pathList->firstNode;
    for (int i = 0; i < game->pathList->size; i++) {
        if (currentNode->site->playerList->lastNode != NULL) {
            break;
        } else {
            currentNode = currentNode->after;
        }   
    }
    return currentNode;
}

/**
 * @brief there is a particular site in front of them, go there.
 * @retval true if yes, false otherwise
 */
bool find_particular_site(Game* game, PathListNode* currentNode, 
        int doIndex, char* siteType) {
    int currentIndex = currentNode->site->siteIndex;
    int barrierIndex = find_next_barrier(game, currentNode);
    PathListNode* nextNode = currentNode->after;
    for (int i = currentIndex; i < game->pathList->size - 1; i++) {
        if (strcmp(nextNode->site->type, siteType) == 0 && nextNode->site->
                playerList->size < nextNode->site->capacity) {
            if (nextNode->site->siteIndex < barrierIndex) {
                doIndex = nextNode->site->siteIndex;
                send_message_do(game, doIndex);
                return true;
            } else {
                doIndex = barrierIndex;
                send_message_do(game, doIndex);
                return true;
            }
        } else {
            nextNode = nextNode->after;
        } 
    }
    return false;
}


/**
 * @brief player decide a move base on player strategy, 
 * then send a message to dealer
 * @retval None
 */
void player_decide_move(Game* game) {
    int doIndex = -1;
    // find current place
    PathListNode* currentNode = find_current_index(game);
    int currentIndex = currentNode->site->siteIndex;
    // Rule 1
    if (game->players[game->runPlayerId]->money > 0) {
        if (find_particular_site(game, currentNode, doIndex, "Do")) {
            return;
        }
    }

    // Rule 2
    if (strcmp(currentNode->after->site->type, "Mo") == 0 && 
            currentNode->after->site->playerList->size 
            < currentNode->after->site->capacity) {
        doIndex = currentNode->after->site->siteIndex;
        send_message_do(game, doIndex);
        return;
    }

    // Rule 3
    PathListNode* nextNode2 = currentNode->after;
    for (int i = currentIndex; i < game->pathList->size - 1; i++) {
        if (strcmp(nextNode2->site->type, "V1") == 0 && nextNode2->site->
                playerList->size < nextNode2->site->capacity) {
            doIndex = nextNode2->site->siteIndex;
            break;
        } else if (strcmp(nextNode2->site->type, "V2") == 0 && nextNode2->
                site->playerList->size < nextNode2->site->capacity) {
            doIndex = nextNode2->site->siteIndex;
            break;
        } else if (strcmp(nextNode2->site->type, "::") == 0 && nextNode2->
                site->playerList->size < nextNode2->site->capacity) {
            doIndex = nextNode2->site->siteIndex;
            break;
        } else {
            nextNode2 = nextNode2->after;
        }   
    }
    send_message_do(game, doIndex);
    return;
}
