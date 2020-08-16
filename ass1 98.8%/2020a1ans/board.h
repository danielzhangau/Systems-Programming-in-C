#ifndef BOARD_H
#define BOARD_H

#include <stdbool.h>
#include <stdio.h>
#include "errs.h"

#define BLANK '.'
#define P0_CHAR 'O'
#define P1_CHAR 'X'

typedef int Dim;

typedef struct {
    int height, width;          // dimensions including the borders
    unsigned short** points;
    char** tokens;
    char turn;
} Board;

// Initialise the already allocated Board struct
// The Dimensions include the borders
void init_board(Board* board, Dim height, Dim width);

// free memory from pointers in Board but not the board itself
void clean_board(Board* board);

// Make dest a copy of src (does not allocate dest)
void clone_board(Board* dest, Board* src);

// True iff the coordinates are valid AND the cell they describe is empty
bool occupied(Board* board, Dim row, Dim column);

// Could a stone be placed at those coordinates?
// This is not just occupied? because it also checks for legal pushing
bool can_place(Board* board, Dim row, Dim column);

// True iff the coordinates are in the borders and a push from that cell 
// is legal
// If there is a legal push, the coordinates of the empty cell to 
// push "into" are passed back via blankRow and blankCol
bool can_push(Board* board, Dim row, Dim column, Dim* blankRow,
        Dim* blankCol);

// Carry out a push from the given coordinates
void push_from(Board* board, Dim row, Dim column);

// Calculate scores for players:
// Player O are passed back in scores[0]
// Player X are passed back in scores[1]
void score(Board* board, unsigned int scores[2]);

// Print the board to the specified FILE
void show_board(Board* board, FILE* out);

// Save the current game statue to a file called fname
bool save_game(Board* board, char* fname);

// Load game state from the file into board
// If successful return OK, otherwise returns an error code
Errs load_game(const char* fname, Board* board);

// Play a stone for the current player in the specified cell
void place(Board* board, Dim r, Dim c);

// True iff the specified cell is on the edge of the board
bool is_border(Board* board, Dim row, Dim column);

// Return the symbol for the current player (current is 0 or 1)
char player_symbol(char current);
#endif
