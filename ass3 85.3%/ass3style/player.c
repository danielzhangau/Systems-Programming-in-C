#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "shared.h"
#include "player.h"
#include "playerStrategy.h"

#define BLANK_CHAR ' '

/** An enum
 *  Define exit status 
 */
typedef enum {
    NORMAL_END = 0,
    INCORRECT_NUMBER = 1,
    INVALID_COUNT = 2,
    INVALID_ID = 3,
    INVALID_PATH = 4,
    EARILY_END = 5,
    COMMUNICATION_ERROR = 6,
} Status;

/** Output error message for status and return status
 *	- Returns nothing
 *	- Prints exit status messages out to stderr(Standard error) 
 */
Status exit_message(Status s) {
    const char* messages[] = {"",                  //0
            "Usage: player pcount ID\n",           //1
            "Invalid player count\n",              //2
            "Invalid ID\n",                        //3
            "Invalid path\n",                      //4
            "Early game over\n",                   //5
            "Communications error\n"};             //6
    fputs(messages[s], stderr);
    return s;
}

/**
 * @brief  loads the path from file at path game->pathfile. 
 * @note   requires that game->pathfile is set. Execution may result in process
 *      terminating with PATH_READ_FAIL
 * @param  game: the game struct
 * @retval None
 */
Status load_path(Game* game) {
    char buffer[BUFFER_SIZE], symbol, site[2], mi; // symbol: ';'   mi: '-' 
    if (!fgets(buffer, BUFFER_SIZE, stdin)) { // Get the whole deck line
        return COMMUNICATION_ERROR;
    }
    unsigned int numOfSites, a;// a: the capacity of the site--use later
    if ((sscanf(buffer, "%u", &numOfSites) != 1 || numOfSites < 2) && 
            (sscanf(buffer, "%c", &symbol) != 1 || symbol != ';')) { 
        return INVALID_PATH;    
    }
    char* restBuffer = buffer + 2;
    if (numOfSites >= 10) {
        restBuffer = restBuffer + 1;
        if (buffer[3 + numOfSites * 3] != '\n') { //check unexpected eof
            return INVALID_PATH;
        }
    } else if ((numOfSites < 10) && (buffer[2 + numOfSites * 3] != '\n')) {
        return INVALID_PATH;
    }
    
    game->pathList = path_create(); // Create Pathlist and populate
    for (unsigned int i = 0; i < numOfSites; i++) {  
        if (sscanf(restBuffer, "%2s", site) != 1) {
            return INVALID_PATH;     
        }
        restBuffer = restBuffer + 2;
        if ((i == 0 || i == numOfSites - 1) && (strcmp(site, "::") != 0)) {  
            return INVALID_PATH;       // check first, last
        }
        if ((strcmp(site, "::") == 0) && 
                (sscanf(restBuffer, "%c", &mi) != 1 || mi != '-')) {
            return INVALID_PATH;      
        } else if ((strcmp(site, "::") != 0) && (sscanf
                (restBuffer, "%u", &a) != 1 || !valid_site(site, a))) {
            return INVALID_PATH;
        }

        restBuffer = restBuffer + 1;
        Site* newSite = (Site*)malloc(sizeof(Site));
        strcpy(newSite->type, site);
        if (strcmp(site, "::") == 0) {
            newSite->capacity = game->numberOfPlayers;
        } else {
            newSite->capacity = a;
        }
        newSite->siteIndex = i;
        newSite->playerList = player_create();
        path_add_site(game->pathList, newSite);
    }
    return NORMAL_END;
} 

/**
 * @brief  parses a HAP message. Playerid in the message will make a move
 *      and update player information
 * @note   can terminate with code COMMUNICATION_ERROR
 * @param  message: HAPp,n,s,m,c from dealer
 * @retval communication status
 */
Status parse_message_hap(Game* game, char* message) {
    if (isspace(message[0])) { 
        return COMMUNICATION_ERROR; // strtoul allows leading spaces
    }
    char* playerIdErr;  // get playerId in message
    unsigned int playerIdInMessage = strtoul(message, &playerIdErr, 10);
    if (playerIdErr == &message[0]) {
        return COMMUNICATION_ERROR;
    }
    game->hapPlayerId = playerIdInMessage;
    message = playerIdErr; // parse other 4 number
    unsigned int newSite, addedPoint, addedMoney, cardIndex;
    if (sscanf(message, ",%u,%u,%u,%u", &newSite, &addedPoint, &addedMoney, 
            &cardIndex) != 4 || newSite >= game->pathList->size || 
            newSite < 0 || addedPoint < 0 || cardIndex < 0 || cardIndex > 5) {
        return COMMUNICATION_ERROR;
    }

    // playerId move to newsite number
    player_make_move(game, newSite);
    Player* playerInMessage = game->players[playerIdInMessage];
    playerInMessage->points += addedPoint;
    playerInMessage->money += addedMoney;
    if (cardIndex != 0) {
        list_add_card(playerInMessage->hand, cardIndex);
    }
    if (cardIndex == 1) {
        playerInMessage->countA += 1;
    } else if (cardIndex == 2) {
        playerInMessage->countB += 1;
    } else if (cardIndex == 3) {
        playerInMessage->countC += 1;
    } else if (cardIndex == 4) {
        playerInMessage->countD += 1;
    } else if (cardIndex == 5) {
        playerInMessage->countE += 1;
    }
    // Print player information
    fprintf(stderr, "Player %d Money=%d V1=%d V2=%d Points=%d A=%d "
            , playerIdInMessage, playerInMessage->money, 
            playerInMessage->countV1, playerInMessage->countV2, 
            playerInMessage->points, playerInMessage->countA);
    fprintf(stderr, "B=%d C=%d D=%d E=%d\n", 
            playerInMessage->countB, playerInMessage->countC, 
            playerInMessage->countD, playerInMessage->countE);
    fflush(stderr);
    // reprint the game path and players
    print_path(stderr, game);
    print_player(stderr, game);
    return NORMAL_END;
}

/**
 * @brief  player move to the given location, update the board node info
 * @param  siteIndex: the site index playerId move to
 * @retval none
 */
void player_make_move(Game* game, int siteIndex) {
    // we remove the id from current place
    PathListNode* nextNode = game->pathList->firstNode;
    for (int i = 0; i < game->pathList->size; i++) {
        if (nextNode->site->playerList->lastNode != NULL) {
            list_remove_player(nextNode->site->playerList);
            break;
        } else {
            nextNode = nextNode->after;
        }   
    }
    
    // then move to new site index
    PathListNode* nextNode2 = game->pathList->firstNode;
    PlayerList* playerList;
    for (int i = 0; i < game->pathList->size; i++) {
        if (nextNode2->site->siteIndex == siteIndex) {
            playerList = nextNode2->site->playerList;
            break;
        } else {
            nextNode2 = nextNode2->after;
        }
    }
    player_add_id(playerList, game->hapPlayerId);
    if (strcmp(nextNode2->site->type, "V1") == 0) {
        game->players[game->hapPlayerId]->countV1 += 1;
    } else if (strcmp(nextNode2->site->type, "V2") == 0) {
        game->players[game->hapPlayerId]->countV2 += 1;
    }
}

/**
 * @brief  send a DO message. dealer will move the playerId to 
 *      the given position
 * @param  position: player decided index
 * @retval none
 */
void send_message_do(Game* game, int position) {
    fprintf(stdout, "DO%d\n", position);
    fflush(stdout);
    return;
}

/**
 * @brief  player->Count A B C D E minus one respectively
 * @param  player: to specific player in game
 * @retval none
 */
void minus_count(Player* player) {
    if (player->countA > 0) {
        player->countA -= 1;
    }
    if (player->countB > 0) {
        player->countB -= 1;
    }
    if (player->countC > 0) {
        player->countC -= 1;
    }
    if (player->countD > 0) {
        player->countD -= 1;
    }
    if (player->countE > 0) {
        player->countE -= 1;
    }
}

/**
 * @brief  calculate each player score base on points, moneys, card counts
 *      and number of visit on V1 and V2
 * @retval none
 */
void calculate_score(Game* game) {
    for (int i = 0; i < game->numberOfPlayers; i++) {
        Player* player = game->players[i];
        // Each complete set of ABCDE is worth ten points.
        while (player->countA > 0 && player->countB > 0 && player->countC > 0 
                && player->countD > 0 && player->countE > 0) {
            player->points += 10;
            minus_count(player);
        }
        // Each set of four types is worth seven points.
        while (((player->countA > 0) + (player->countB > 0) + (player->countC 
                > 0) + (player->countD > 0) + (player->countE > 0)) == 4) {
            player->points += 7;
            minus_count(player);
        }
        while (((player->countA > 0) + (player->countB > 0) + (player->countC 
                > 0) + (player->countD > 0) + (player->countE > 0)) == 3) {
            player->points += 5;
            minus_count(player);
        }
        while (((player->countA > 0) + (player->countB > 0) + (player->countC 
                > 0) + (player->countD > 0) + (player->countE > 0)) == 2) {
            player->points += 3;
            minus_count(player);
        } 
        while (((player->countA > 0) + (player->countB > 0) + (player->countC 
                > 0) + (player->countD > 0) + (player->countE > 0)) == 1) {
            player->points += 1;
            minus_count(player);
        } 

        player->points += player->countV1;
        player->points += player->countV2;
    } 
}

/**
 * @brief  print each player score in format
 * @param  output: stderr or stdout
 * @retval none
 */
void print_score(FILE* output, Game* game) {
    fprintf(output, "Scores: ");
    for (int i = 0; i < game->numberOfPlayers; i++) {
        if (i == game->numberOfPlayers - 1) {
            fprintf(output, "%d", game->players[i]->points);
            fputc('\n', output);
        } else {
            fprintf(output, "%d", game->players[i]->points);
            fputc(',', output);
        }
    }
    fflush(output);
}

/**
 * @brief  print game path in format
 * @param  output: stderr or stdout
 * @param  game: current game
 * @retval none
 */
void print_path(FILE* output, Game* game) {
    PathListNode* nextNode = game->pathList->firstNode;
    while(nextNode != NULL) { 
        fputs(nextNode->site->type, output);
        fputc(' ', output);
        nextNode = nextNode->after;
    }
    fputc('\n', output);
    fflush(output);
}

/**
 * @brief  print each pathlist->playerlist in format
 * @param  output: stderr or stdout
 * @retval none
 */
void print_player(FILE* output, Game* game) {
    // loop to find the greatest player list size
    PathListNode* nextNode = game->pathList->firstNode;
    int greatestPlayerListSize = 0;
    for (int i = 0; i < game->pathList->size; i++) {
        if (nextNode->site->playerList->size > greatestPlayerListSize) {
            greatestPlayerListSize = nextNode->site->playerList->size;
        }
        nextNode = nextNode->after;    
    }
    // initial board
    Board* board = &game->board;
    board->height = greatestPlayerListSize;
    board->width = game->pathList->size;
    board->grid = malloc(sizeof(Cell*) * greatestPlayerListSize);
    for (unsigned i = 0; i < greatestPlayerListSize; ++i) {
        board->grid[i] = malloc(sizeof(Cell) * game->pathList->size);
        for (unsigned j = 0; j < game->pathList->size; ++j) {
            board->grid[i][j].pid = BLANK_CHAR;
            board->grid[i][j].space1 = BLANK_CHAR;
            board->grid[i][j].space2 = BLANK_CHAR;
        }
    }

    //add content
    int j = 0;
    PathListNode* nextNode2 = game->pathList->firstNode;
    while(nextNode2 != NULL) { 
        PlayerListNode* playerNode = nextNode2->site->playerList->firstNode;
        int i = 0;
        while(playerNode != NULL) {
            board->grid[i][j].pid = playerNode->pid + 48;
            playerNode = playerNode->after;
            i++;
        }
        nextNode2 = nextNode2->after;
        j++;
    }

    // print grid
    for (unsigned row = 0; row < board->height; ++row) {
        for (unsigned col = 0; col < board->width; ++col) {
            fputc(board->grid[row][col].pid, output);
            fputc(board->grid[row][col].space1, output);
            fputc(board->grid[row][col].space2, output);
        }
        fputc('\n', output);
    }
    free(board->grid);
    fflush(output);
}

/**
 * @brief  check if string is end with '\n'
 * @note   can terminate with code COMMUNICATION_ERROR
 * @param  message: should be '\n'
 * @retval communication status
 */
Status check_string_eof(char* message) {
    if (message[0] == '\0') {
        return COMMUNICATION_ERROR; // unexpected EOF
    }
    if (message[0] != '\n') {
        return COMMUNICATION_ERROR;
    }
    return NORMAL_END;
}

/**
 * @brief  runs the main loop of the game, parse message from dealer
 *      and make actions accordingly
 * @retval communication status
 */
Status play_game(Game* game) {
    print_path(stderr, game);
    print_player(stderr, game);

    char buffer[BUFFER_SIZE];
    // run loop until error or EARILY_END or DONE message recieved
    while (true) {
        if(!fgets(buffer, BUFFER_SIZE, stdin)) {
            return COMMUNICATION_ERROR; // unexpected EOF
        }

        // handle recieve message
        char* strAfterParse;
        if (!strncmp("YT", buffer, 2)) {
            // check remainder of message
            Status status = check_string_eof(buffer + 2);
            if (status != NORMAL_END) {
                return status;
            }
            player_decide_move(game);
        } else if (!strncmp("EARLY", buffer, 5)) {
            Status status = check_string_eof(buffer + 5);
            if (status != NORMAL_END) {
                return status;
            }
            return EARILY_END;
        } else if (!strncmp("DONE", buffer, 4)) {
            Status status = check_string_eof(buffer + 4);
            if (status != NORMAL_END) {
                return status;
            }
            calculate_score(game);
            print_score(stderr, game);
            return NORMAL_END;
        } else if (!strncmp("HAP", buffer, 3)) {
            Status status = parse_message_hap(game, buffer + 3);
            if (status != NORMAL_END) {
                return status;
            }
            strAfterParse = buffer + 3; // HAP
            strAfterParse = strAfterParse + strlen(buffer + 3) - 1;
            Status afterStatus = check_string_eof(strAfterParse);
            if (afterStatus != NORMAL_END) {
                return afterStatus;
            }
        } else {
            return COMMUNICATION_ERROR; // unknown/malformed message header
        }
    }
}

/**
 * @brief  intialize player information
 * @note   each player start with 7 money
 * @param  playerNumber aka playerId
 * @retval player
 */
Player* start_player_process(Game* game, int playerNumber) {
    Player* player = (Player*)malloc(sizeof(Player));
    player->playerId = playerNumber;
    player->hand = list_create();
    player->points = 0;
    player->money = 7;
    player->countV1 = 0;
    player->countV2 = 0;
    player->countA = 0;
    player->countB = 0;
    player->countC = 0;
    player->countD = 0;
    player->countE = 0;
    return player;
}

int main(int argc, char const *argv[]) {
    // check number of arguments is correct
    if (argc != 3) {
        return exit_message(INCORRECT_NUMBER);
    }
    Game* game = (Game*)malloc(sizeof(Game));
    game->players = (Player**)malloc(sizeof(Player) * game->numberOfPlayers);

    // load number of players
    char* numberOfPlayersError;
    game->numberOfPlayers = strtol(argv[1], &numberOfPlayersError, 10);
    if (*numberOfPlayersError != '\0' || game->numberOfPlayers < 1) {
        return exit_message(INVALID_COUNT);
    }

    // load player number
    if (strcmp(argv[2], "") == 0) {
        return exit_message(INVALID_ID);
    }
    char* playerIdError;
    game->runPlayerId = strtol(argv[2], &playerIdError, 10);
    if (*playerIdError != '\0' || game->runPlayerId < 0 || 
            game->runPlayerId >= game->numberOfPlayers) {
        return exit_message(INVALID_ID);
    }
    for (int i = 0; i < game->numberOfPlayers; i++) {
        // initialize player information 
        Player* newPlayer = start_player_process(game, i);
        game->players[i] = newPlayer;
    }

    // send startup confirmation to 2310dealer
    fputc('^', stdout);
    fflush(stdout);

    // Load path
    Status pathStatus = load_path(game);
    if (pathStatus != NORMAL_END) {
        return exit_message(pathStatus);
    }

    // add player into the game  FILO e.g 7 6 5 4 3 2 1<- last node
    for (int i = game->numberOfPlayers - 1; i >= 0; --i) {
        player_add_id(game->pathList->firstNode->site->playerList, i);
    }

    return exit_message(play_game(game));
}

