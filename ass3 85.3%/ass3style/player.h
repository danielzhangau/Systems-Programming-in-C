#ifndef PLAYER_H_
#define PLAYER_H_
#include <stdio.h>
#include <stdlib.h>
#include "shared.h"

#define BLANK_CHAR ' '  // use for print playerId

typedef struct {
    int pid;
    char space1;  // use for print playerId
    char space2;
} Cell;

typedef struct {
    unsigned int height, width;
    Cell** grid;
} Board;

typedef struct {
    unsigned int playerId;
    CardList* hand;
    int points;
    int money;
    int countV1;
    int countV2;
    int countA;
    int countB;
    int countC;
    int countD;
    int countE;
} Player;

typedef struct {
    int numberOfPlayers;
    unsigned int hapPlayerId;
    int runPlayerId;
    Board board;
    PathList* pathList;
    Player** players;
} Game;

void player_make_move(Game* game, int siteIndex);

void send_message_do(Game* game, int position);

void minus_count(Player* player);

void calculate_score(Game* game);

void print_score(FILE* output, Game* game);

void print_path(FILE* output, Game* game);

void print_player(FILE* output, Game* game);

Player* start_player_process(Game* game, int playerNumber);

#endif