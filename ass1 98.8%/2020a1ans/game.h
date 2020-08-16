#ifndef GAME_H
#define GAME_H

#include "board.h"

// type for functions which choose a move for a player
// The move is sent back via the Dim parameters 
// (row and column respectively)
typedef bool (*MoveFn)(Board*, Dim*, Dim*);

typedef struct {
    Board* board;
    MoveFn moves[2];
    PType types[2];
} Game;

// Set up the (already allocated) game with the corresponding player types
void init_game(Game* game, PType zero, PType one);

// Free all memory linked to the game (except the Game struct itself)
void clean_game(Game* game);

// return true if buffer contains any new input
// The params allow for buffers to be reused and resized
bool get_line(char** buffer, size_t* capacity);

// doesn't show board
// return false on player EOF
bool get_human_move(Board* board, Dim* r, Dim* c);
// do not call on a full board
bool get_simple_move(Board* board, Dim* r, Dim* c);

bool would_decrease(Board* board, Dim r, Dim c, unsigned int scoreNow);

bool get_better_move(Board* board, Dim* r, Dim* c);
bool centre_full(Board* board);
Errs run_game(Game* g);
#endif
