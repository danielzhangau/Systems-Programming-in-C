#include <stdlib.h>
#include <string.h>
#include "game.h"

// Set up the (already allocated) game with the corresponding player types
void init_game(Game* game, PType zero, PType one)
{
    MoveFn fns[] = {get_simple_move, get_better_move, get_human_move, 0};
    game->board = malloc(sizeof(Board));
    game->moves[0] = fns[zero];
    game->moves[1] = fns[one];
    game->types[0] = zero;
    game->types[1] = one;
}

// Free all memory linked to the game (except the Game struct itself)
void clean_game(Game* game)
{
    clean_board(game->board);
    free(game->board);
}

// return true if buffer contains any new input
// The params allow for buffers to be reused and resized
bool get_line(char** buffer, size_t* capacity)
{
    size_t current = 0;
    (*buffer)[0] = '\0';
    int input;
    while (input = fgetc(stdin), input != EOF && input != '\n') {
        if (current > *capacity - 2) {
            size_t newCap = (size_t) (*capacity * 1.5);
            void* newBlock = realloc(*buffer, newCap);
            if (newBlock == 0) {
                return false;
            }
            *capacity = newCap;
            *buffer = newBlock;
        }
        (*buffer)[current] = (char)input;
        (*buffer)[++current] = '\0';
    }
    return input != EOF;
}

// doesn't show board
// return false on player EOF
bool get_human_move(Board* board, Dim* r, Dim* c)
{
    char* buffer = malloc(sizeof(char) * 40);
    size_t cap = 40;
    *r = 0;
    *c = 0;
    bool res = false;
    while (!res) {
        printf("%c:(R C)> ", player_symbol(board->turn));
        if (get_line(&buffer, &cap)) {
            if ((buffer[0] == 's') && (strlen(buffer) > 1)) {
                if (!save_game(board, buffer + 1)) {
                    fputs("Save failed\n", stderr);
                }
                continue;
            }
            char* err1, *err2;
            *r = strtol(buffer, &err1, 10);
            if (*err1 != ' ') {
                continue;
            }
            *c = strtol(err1 + 1, &err2, 10);
            if (*err2 != '\0') {
                continue;
            }
        } else {                // couldn't read a line
            res = false;        // so end the loop
            break;
        }
        res = can_place(board, *r, *c);
    }
    free(buffer);
    return res;
}

// do not call on a full board
bool get_simple_move(Board* board, Dim* r, Dim* c)
{
    Dim cStart = 1, cEnd = board->width - 1;
    Dim rStart = 1, rEnd = board->height - 1;
    short rDelta = 1, cDelta = 1;
    if (board->turn) {
        cStart = board->width - 2, cEnd = 0;
        rStart = board->height - 2, rEnd = 0;
        rDelta = -1, cDelta = -1;
    }
    for (*r = rStart; *r != rEnd; *r += rDelta) {
        for (*c = cStart; *c != cEnd; *c += cDelta) {
            if (can_place(board, *r, *c)) {
                return true;
            }
        }
    }
    return false;               // should never happen
}

// True iff the current player placing at the given coordinates would
// give the other player a lower score than scoreNow
bool would_decrease(Board* board, Dim r, Dim c, unsigned int scoreNow)
{
    if (!can_place(board, r, c)) {
        return false;
    }
    Board b;
    clone_board(&b, board);
    push_from(&b, r, c);
    unsigned int scores[2];
    score(&b, scores);
    return (scores[board->turn ^ 1] < scoreNow);
}

// Generate a move for a Type one player
// do not call on a full board
bool get_better_move(Board* board, Dim* r, Dim* c)
{
    // first find the opponent's current score
    unsigned int scores[2];
    score(board, scores);

    // check border regions first
    *r = 0;
    for (*c = 1; *c < board->width; (*c)++) {
        if (would_decrease(board, *r, *c, scores[board->turn ^ 1])) {
            return true;
        }
    }
    *c = board->width - 1;
    for (*r = 1; *r < board->height - 1; (*r)++) {
        if (would_decrease(board, *r, *c, scores[board->turn ^ 1])) {
            return true;
        }
    }
    *r = board->height - 1;
    for (*c = board->width - 2; *c > 0; (*c)--) {
        if (would_decrease(board, *r, *c, scores[board->turn ^ 1])) {
            return true;
        }
    }
    *c = 0;
    for (*r = board->height - 1; *r > 0; (*r)--) {
        if (would_decrease(board, *r, *c, scores[board->turn ^ 1])) {
            return true;
        }
    }
    Dim maxR = 0, maxC = 0;
    unsigned short maxScore = 0;
    // nothing in the border regions so look for the biggest free points
    for (*r = 1; *r < board->height - 1; (*r)++) {
        for (*c = 1; *c < board->width - 1; (*c)++) {
            if ((board->points[*r][*c] > maxScore)
                    && (board->tokens[*r][*c] == BLANK)) {
                maxR = *r;
                maxC = *c;
                maxScore = board->points[*r][*c];
            }
        }
    }
    if (maxScore > 0) {
        *r = maxR;
        *c = maxC;
        return true;
    }
    return false;
}

//Is the interior of the board full?
bool centre_full(Board* board)
{
    for (Dim r = 1; r < board->height - 1; ++r) {
        for (Dim c = 1; c < board->width - 1; ++c) {
            if (board->tokens[r][c] == BLANK) {
                return false;
            }
        }
    }
    return true;
}

// Play a loaded game 
// returns OK on a normal game over, other error code in other cases
Errs run_game(Game* g)
{
    if (centre_full(g->board)) {
        return FULLBOARD;
    }
    show_board(g->board, stdout);
    do {
        Dim r, c;
        if (!g->moves[(size_t) g->board->turn](g->board, &r, &c)) {
            return PLAYEREOF;
        }
        place(g->board, r, c);
        if (g->types[(size_t) g->board->turn] != HUMAN) {
            fprintf(stdout, "Player %c placed at %u %u\n",
                    player_symbol(g->board->turn), r, c);
        }
        show_board(g->board, stdout);
        g->board->turn = (g->board->turn + 1) % 2;
    } while (!centre_full(g->board));
    // who won?
    unsigned int scores[2];
    score(g->board, scores);
    printf("Winners: ");
    if (scores[0] > scores[1]) {
        putc(player_symbol(0), stdout);
    } else if (scores[0] < scores[1]) {
        putc(player_symbol(1), stdout);
    } else {
        printf("%c %c", player_symbol(0), player_symbol(1));
    }
    putc('\n', stdout);
    return OK;
}
