#include <stdlib.h>
#include <stdio.h>
#include "errs.h"
#include "board.h"

// Initialise the already allocated Board struct
// The Dimensions include the borders
void init_board(Board* board, Dim height, Dim width)
{
    board->height = height;
    board->width = width;
    board->points = malloc(sizeof(unsigned short*) * height);
    board->tokens = malloc(sizeof(char*) * height);
    for (Dim i = 0; i < height; ++i) {
        board->points[i] = malloc(sizeof(unsigned short) * width);
        board->tokens[i] = malloc(sizeof(char) * width);
    }
    for (Dim r = 0; r < board->height; ++r) {
        for (Dim c = 0; c < board->width; ++c) {
            board->points[r][c] = is_border(board, r, c) ? 0 : 1;
            board->tokens[r][c] = BLANK;
        }
    }
}

// free memory from pointers in Board but not the board itself
void clean_board(Board* board)
{
    for (Dim r = 0; r < board->height; ++r) {
        free(board->points[r]);
        free(board->tokens[r]);
    }
    free(board->points);
    free(board->tokens);
}

// Make dest a copy of src (does not allocate dest)
void clone_board(Board* dest, Board* src)
{
    init_board(dest, src->height, src->width);
    dest->turn = src->turn;
    for (Dim r = 0; r < src->height; ++r) {
        for (Dim c = 0; c < src->width; ++c) {
            dest->tokens[r][c] = src->tokens[r][c];
            dest->points[r][c] = src->points[r][c];
        }
    }
}

// True iff row, column is a valid cell on the board
bool valid(Board* board, Dim row, Dim column)
{
    if ((row < 0) || (column < 0) || (row >= board->height) ||
            (column >= board->width)) {
        return false;
    }
    // check for corners
    if ((row == 0) && (column == 0 || column == board->width - 1)) {
        return false;
    }
    if ((row == board->height - 1)
            && (column == 0 || column == board->width - 1)) {
        return false;
    }
    return true;
}

// True iff the coordinates are valid AND the cell they describe is not blank
bool occupied(Board* board, Dim row, Dim column)
{
    if (!valid(board, row, column)) {
        return false;
    }
    return (board->tokens[row][column] != BLANK);
}

// True iff the coordinates are on the edge of the board
bool is_border(Board* board, Dim row, Dim column)
{
    if ((row == 0) && (column == 0 || column == board->width - 1)) {
        return false;
    }
    if ((row == board->height - 1)
            && (column == 0 || column == board->width - 1)) {
        return false;
    }
    return ((row == 0) || (row == board->height - 1) || (column == 0)
            || (column == board->width - 1));
}

// helper function for can_push (takes the same parameters)
bool can_push_up_down(Board* board, Dim row, Dim column, Dim* blankRow,
        Dim* blankCol)
{
    if (row == 0) {
        if (board->tokens[1][column] == BLANK) {
            return false;
        }
        for (Dim r = 1; r < board->height; ++r) {
            if (board->tokens[r][column] == BLANK) {
                *blankRow = r;// TAG:11
                *blankCol = column;// TAG:12
                return true;
            }
        }
        return false;
    }
    if (row == board->height - 1) {// TAG:13
        if (board->tokens[row - 1][column] == BLANK) {// TAG:14
            return false;
        }
        for (Dim r = row - 1; r >= 0; --r) {
            if (board->tokens[r][column] == BLANK) {// TAG:15
                *blankRow = r;// TAG:16
                *blankCol = column;// TAG:17
                return true;
            }
        }
        return false;
    }
    return false;
}

// helper function for can_push (takes the same paramters)
bool can_push_left_right(Board* board, Dim row, Dim column, Dim* blankRow,
        Dim* blankCol)
{
    if (column == 0) {
        if (board->tokens[row][1] == BLANK) {// TAG:18
            return false;
        }
        for (Dim c = 1; c < board->width; ++c) {
            if (board->tokens[row][c] == BLANK) {
                *blankRow = row;// TAG:19
                *blankCol = c;// TAG:20
                return true;
            }
        }
        return false;
    }
    if (column == board->width - 1) {
        if (board->tokens[row][column - 1] == BLANK) {
            return false;
        }
        for (Dim c = column - 1; c >= 0; --c) {
            if (board->tokens[row][c] == BLANK) {
                *blankRow = row;// TAG:21
                *blankCol = c;// TAG:22
                return true;
            }
        }
        return false;
    }
    return false;
}


// can push from this location
// Is there a blank space somewhere in the correct direction
//   and the next cell in the correct direction is not blank
// Precondition: only called on border cells
bool can_push(Board* board, Dim row, Dim column, Dim* blankRow,
        Dim* blankCol)
{
    if (can_push_up_down(board, row, column, blankRow, blankCol)) {
        return true;
    }
    return can_push_left_right(board, row, column, blankRow, blankCol);
}

// Could a stone be placed at those coordinates?
// This is not just occupied? because it also checks for legal pushing
bool can_place(Board* board, Dim row, Dim column)
{                               // check for placement rules
    if (!valid(board, row, column)) {
        return false;
    }
    if (occupied(board, row, column)) {
        return false;
    }
    if (!is_border(board, row, column)) {
        return true;
    }
    Dim dummy1, dummy2;
    return can_push(board, row, column, &dummy1, &dummy2);
}

// Carry out a push from the given coordinates
void push_from(Board* board, Dim row, Dim column)
{
    Dim blankRow, blankCol;
    if (!can_push(board, row, column, &blankRow, &blankCol)) {
        return;
    }
    board->tokens[row][column] = player_symbol(board->turn);
    int deltaR = 0, deltaC = 0;
    if (row > blankRow) {
        deltaR = 1;
    } else if (row < blankRow) {
        deltaR = -1;
    } else if (column < blankCol) {
        deltaC = -1;
    } else {
        deltaC = 1;
    }
    // starting with the blank cell, copy the "previous" contents
    // moving backwards to push point
    Dim currR = blankRow, currC = blankCol;
    while ((currR != row) || (currC != column)) {
        Dim newR = currR + deltaR;
        Dim newC = currC + deltaC;
        board->tokens[currR][currC] = board->tokens[newR][newC];
        board->tokens[newR][newC] = BLANK;
        currR = newR;
        currC = newC;
    }
}

// Calculate scores for players:
// Player O are passed back in scores[0]
// Player X are passed back in scores[1]
void score(Board* board, unsigned int scores[2])
{
    scores[0] = 0;
    scores[1] = 0;
    for (Dim r = 0; r < board->height; ++r) {
        for (Dim c = 0; c < board->width; ++c) {
            if (board->tokens[r][c] == P0_CHAR) {
                scores[0] += board->points[r][c];
            } else if (board->tokens[r][c] == P1_CHAR) {
                scores[1] += board->points[r][c];
            }
        }
    }
}

// Print the board to the specified FILE
void show_board(Board* board, FILE* out)
{
    for (Dim r = 0; r < board->height; ++r) {
        for (Dim c = 0; c < board->width; ++c) {
            if (valid(board, r, c)) {
                fprintf(out, "%hu%c", board->points[r][c],
                        board->tokens[r][c]);
            } else {
                fputs("  ", out);
            }
        }
        fputs("\n", out);
    }
    fflush(stdout);
}

// Save the current game statue to a file called fname
bool save_game(Board* board, char* fname)
{
    FILE* out = fopen(fname, "w");
    if (!out) {
        return false;
    }
    fprintf(out, "%d %d\n", board->height, board->width);
    fprintf(out, "%c\n", board->turn ? P1_CHAR : P0_CHAR);
    show_board(board, out);
    fclose(out);
    return true;
}

// Read board contents from already open FILE
// This assumes that board has already been inited
bool load_board(FILE* in, Board* board) {
    int input1, input2;
    for (Dim r = 0; r < board->height; ++r) {
        for (Dim c = 0; c < board->width; ++c) {
            input1 = fgetc(in);
            input2 = fgetc(in);
            if (!valid(board, r, c)) {
                if ((input1 == ' ') && (input2 == ' ')) {
                    continue;
                }
                fclose(in);
                clean_board(board);
                return false;
            }
            if (input1 == EOF || input2 == EOF || input1 < '0' 
                    || input1 > '9'
                    || (input2 != BLANK && input2 != P0_CHAR && 
                    input2 != P1_CHAR)) {
                clean_board(board);
                fclose(in);
                return false;
            }
            board->points[r][c] = input1 - '0';
            board->tokens[r][c] = input2;
            // all interior should have points
            if (!is_border(board, r, c) && board->points[r][c] == 0) {
                clean_board(board);
                fclose(in);
                return false;
            }
        }
        input1 = fgetc(in);
        if (input1 != '\n') {
            clean_board(board);
            fclose(in);
            return false;
        }
    }
    return true;
}

// Load game state from the file into board
// If successful return OK, otherwise returns an error code
Errs load_game(const char* fname, Board* board)
{
    FILE* in = fopen(fname, "r");
    if (!in) {
        return NOFILE;
    }
    int height, width;
    char dummy1, dummy2;
    char turn;
    if ((fscanf(in, "%d%c%d%c", &height, &dummy1,
            &width, &dummy2) != 4) ||
            (dummy1 != ' ') || (dummy2 != '\n') || (height < 3) ||
            (width < 3)) {
        fclose(in);
        return FILECONTENTS;
    }
    if ((fscanf(in, "%c%c", &turn, &dummy1) != 2) ||
            ((turn != 'O') && (turn != 'X')) ||
            dummy1 != '\n') {
        fclose(in);
        return FILECONTENTS;
    }
    init_board(board, height, width);
    board->turn = (turn == P1_CHAR);
    if (!load_board(in, board)) {
        return FILECONTENTS;
    }
    if (fgetc(in) != EOF) {
        clean_board(board);
        fclose(in);
        return FILECONTENTS;
    }
    fclose(in);
    return OK;
}

// Return the symbol for the current player (current is 0 or 1)
char player_symbol(char current)
{
    return current ? P1_CHAR : P0_CHAR;
}

// Play a stone for the current player in the specified cell
void place(Board* board, Dim r, Dim c)
{
    if (is_border(board, r, c)) {
        push_from(board, r, c);
    } else {
        board->tokens[r][c] = player_symbol(board->turn);
    }
}
