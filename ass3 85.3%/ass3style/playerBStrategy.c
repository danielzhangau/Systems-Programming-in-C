#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "player.h"
#include "shared.h"

/**
 * @brief  check if the player have most card
 * @param  playerId: specific player
 * @retval true if yes, false otherwise
 */
bool has_most_card(Game* game, int playerId) {
    int greatestCardNumber = 0;
    int greatestId = 0;
    for (int i = 0; i < game->numberOfPlayers; i++) {
        if (game->players[i]->hand->size > greatestCardNumber) {
            greatestCardNumber = game->players[i]->hand->size;
            greatestId = i;
        }
    }
    // check if at lease two player have "most"
    for (int i = 0; i < game->numberOfPlayers; i++) {
        if (game->players[i]->hand->size == greatestCardNumber && 
                i != greatestId) {
            return false;
        }
    }
    if (game->players[playerId]->hand->size == greatestCardNumber) {
        return true;
    }
    return false;
}

/**
 * @brief  check if all player have zero card
 * @retval true if yes, false otherwise
 */
bool all_zero_card(Game* game) {
    for (int i = 0; i < game->numberOfPlayers; i++) {
        if (game->players[i]->hand->size > 0) {
            return false;
        }
    }
    return true;
}

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
 * @brief check if all other players are on later sites than us,
 * @retval true if yes, false otherwise
 */
bool all_node_later(Game* game, PathListNode* currentNode) {
    int currentIndex = currentNode->site->siteIndex;
    int playerNumber = 0;
    PathListNode* nextNode = currentNode->after;
    for (int i = currentIndex + 1; i < game->pathList->size; i++) {
        playerNumber += nextNode->site->playerList->size;
        nextNode = nextNode->after;
    }
    if (playerNumber == game->numberOfPlayers - 1) {
        return true;
    } else {
        return false;
    }
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
    for (int i = currentIndex; i < barrierIndex; i++) {
        if (strcmp(nextNode->site->type, siteType) == 0 && nextNode->site->
                playerList->size < nextNode->site->capacity) {
            doIndex = nextNode->site->siteIndex;
            send_message_do(game, doIndex);
            return true;
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
    int runPlayerId = game->runPlayerId;
    int doIndex = -1;
    // find current place
    PathListNode* currentNode = find_current_index(game);
    int currentIndex = currentNode->site->siteIndex;
    int barrierIndex = find_next_barrier(game, currentNode);

    // Rule 1    
    if (currentNode->after->site->playerList->size < currentNode->after->
            site->capacity && all_node_later(game, currentNode)) {
        doIndex = currentNode->after->site->siteIndex;
        send_message_do(game, doIndex);
        return;
    }

    // Rule 2
    if (game->players[runPlayerId]->money % 2 == 1) {
        if (find_particular_site(game, currentNode, doIndex, "Mo")) {
            return;
        }
    }

    // Rule 3
    if (has_most_card(game, runPlayerId) || all_zero_card(game)) {
        if (find_particular_site(game, currentNode, doIndex, "Ri")) {
            return;
        }
    }

    // Rule 4
    for (int i = currentIndex; i < barrierIndex; i++) {
        if (find_particular_site(game, currentNode, doIndex, "V2")) {
            return;
        }
    }
    
    // Rule 5
    PathListNode* nextNode = currentNode->after;
    for (int i = currentIndex; i < barrierIndex; i++) {
        if (nextNode->site->playerList->size < nextNode->site->capacity) {
            doIndex = nextNode->site->siteIndex;
            send_message_do(game, doIndex);
            return;
        } else {
            nextNode = nextNode->after;
        }   
    }
}