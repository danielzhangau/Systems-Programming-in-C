#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#define MAXHANDSIZE 6
#define BLANKCHAR '.'
#define BLANKFILECHAR '*'
#define VALIDDIMLOW 3
#define VALIDDIMHIGH 100

typedef enum {
    OK = 0,
    BADARGNUM = 1,
    BADARGTYPE = 2,
    BADDECKFILE = 3,
    BADSAVEFILE = 4,
    SHORTDECK = 5,
    FULLBOARD = 6,
    EOINPUT = 7
} Status;

// Output error message for status and return status
Status show_message(Status s)
{
    const char* messages[] = {"",
            "Usage: bark savefile p1type p2type\n"
            "bark deck width height p1type p2type\n",
            "Incorrect arg types\n",
            "Unable to parse deckfile\n",
            "Unable to parse savefile\n",
            "Short deck\n",
            "Board full\n",
            "End of input\n"};
    fputs(messages[s], stderr);
    return s;
}

typedef struct {
    char rank;
    char suit;
} Card;

typedef struct {
    unsigned int count;
    unsigned int used;
    Card* contents;
} Deck;

typedef struct {
    unsigned int width, height;
    Card** grid;
} Board;



typedef struct {
    unsigned int size;
    Card cards[MAXHANDSIZE];    // only works because the spec fixes this value
} Hand;

typedef struct {
    Deck deck;
    Board board;
    Hand hand[2];
    char types[2];
    unsigned short current;     // will be 0 or 1 
    char* deckname;
    bool emptyBoard;
} Game;

// Check if a board dimension is in bounds 
bool valid_dim(unsigned d) {
    return (d >= VALIDDIMLOW) && (d <= VALIDDIMHIGH);
}

// Allocate memory for board and set all positions to blank
void init_board(Board* board, unsigned int width, unsigned int height)
{
    board->width = width;
    board->height = height;
    board->grid = malloc(sizeof(Card*) * height);
    for (unsigned i = 0; i < height; ++i) {
        board->grid[i] = malloc(sizeof(Card) * width);
        for (unsigned j = 0; j < width; ++j) {
            board->grid[i][j].rank = BLANKCHAR;
            board->grid[i][j].suit = BLANKCHAR;
        }
    }
}

// True if the two chars (rank|suit) describe a valid card
bool valid_card(char c1, char c2)
{
    return !((c1 < '1') || (c1 > '9') || (c2 < 'A') || (c2 > 'Z'));
}

// is c a valid player type?
bool valid_type(char c)
{
    return (c == 'h') || (c == 'a');
}

// Attempt to draw a card from the deck (returned as reference param card)
// return true if successful
bool draw_card(Deck* deck, Card* card)
{
    if (deck->used >= deck->count) {
        return false;
    }
    *card = deck->contents[deck->used];
    deck->used++;
    return true;
}

// initialise a deck from a file
// true if successful
// caller is responsible for closing input
bool load_deck(FILE* input, Deck* deck)
{
    const short buffSize = (short)log10(INT_MAX) + 3;   // +2 to allow for \n\0
    char buffer[buffSize];
    if (!fgets(buffer, buffSize - 1, input)) {
        return false;
    }
    char* err;
    unsigned int decksize = strtoul(buffer, &err, 10);
    if (*err != '\n') {
        return false;
    }

    Card* contents = malloc(sizeof(Card) * decksize);

    deck->count = decksize;
    for (unsigned i = 0; i < decksize; ++i) {
        if (!fgets(buffer, buffSize - 1, input)) {
            free(contents);
            return false;
        }
        if (((buffer[2] != '\n') && (buffer[2] != '\0')) ||
                !valid_card(buffer[0], buffer[1])) {
            free(contents);
            return false;
        }
        contents[i].rank = buffer[0];
        contents[i].suit = buffer[1];
    }
    deck->used = 0;
    deck->contents = contents;
    return true;
}

// init a deck from the file called fname
bool load_deck_file(const char* fname, Deck* deck)
{
    FILE* f = fopen(fname, "r");
    if (!f) {
        return false;
    }
    bool result = load_deck(f, deck);
    fclose(f);
    return result;
}

// Read a Hand of cards from the line of text passed as buffer
// true on success
bool read_hand(const char* buffer, Hand* hand)
{
    hand->size = 0;
    for (unsigned i = 0; i < MAXHANDSIZE; ++i) {
        if ((buffer[2 * i] == '\0') || (buffer[2 * i] == '\n')) {
            return true;
        } else if (valid_card(buffer[2 * i], buffer[2 * i + 1])) {
            hand->cards[i].rank = buffer[2 * i];
            hand->cards[i].suit = buffer[2 * i + 1];
            hand->size++;
        } else {
            return false;
        }
    }
    return true;
}

// Init a board from the given file
// need to pass in the dimensions of the board
// true on success
bool read_board(Board* board, FILE* input, unsigned width, unsigned height)
{
    init_board(board, width, height);
    for (unsigned i = 0; i < height; ++i) {
        for (unsigned j = 0; j < width; ++j) {
            int c1 = fgetc(input);
            int c2 = fgetc(input);
            if (c1 == EOF || c2 == EOF) {
                return false;
            }
            if ((c1 == BLANKFILECHAR) && (c2 == BLANKFILECHAR)) {
                continue;
            } else if (valid_card(c1, c2)) {
                board->grid[i][j].rank = c1;
                board->grid[i][j].suit = c2;
            } else {
                return false;
            }
        }
        if (i < height - 1) {   // end of row, check for trailing chars & skip
            char c = fgetc(input);   // this allows for missing \n on last line
            if (c != '\n') {
                return false;
            }
        }
    }
    return true;
}

// Check if board is empty
bool board_empty(Board* b) {
    for (unsigned int i = 0; i < b->width; ++i) {
        for (unsigned int j = 0; j < b->height; ++j) {
            if (b->grid[i][j].rank != BLANKCHAR) {
                return false;
            }
        }
    }
    return true;
}

// Init a game object from file called fname
// return OK on success or error status otherwise
Status load_saved(const char* fname, Game* game)
{
    const int buffSize = 250;   // Hopefully big enough to hold paths
    char buffer[buffSize];
    FILE* input = fopen(fname, "r");
    if (!input) {
        return BADSAVEFILE;
    }
    if (!fgets(buffer, buffSize, input)) {
        fclose(input);
        return BADSAVEFILE;
    }
    unsigned int width, height, drawn;
    char next;
         // doesn't check for trailing chars
    if ((sscanf(buffer, "%u %u %u %c", &width, &height, &drawn, &next) != 4) ||
            (next < '1' || next > '2' || !fgets(buffer, buffSize, input))
            || !valid_dim(width) || !valid_dim(height)) {
        fclose(input);
        return BADSAVEFILE;
    }
    buffer[strlen(buffer) - 1] = '\0';
    if (!load_deck_file(buffer, &(game->deck))) {
        fclose(input);
        return BADDECKFILE;
    }
    game->deckname = malloc(sizeof(char) * (strlen(buffer) + 1));
    strcpy(game->deckname, buffer);
    for (int i = 0; i < 2; ++i) {
        if (!fgets(buffer, buffSize, input) ||
                !read_hand(buffer, &game->hand[i])) {
            fclose(input);
            return BADSAVEFILE;
        }
    }
    if ((game->hand[(unsigned)(next - '1')].size != MAXHANDSIZE) ||
            (game->hand[(unsigned)((next - '1' + 1) % 2)].size 
            != MAXHANDSIZE - 1)) {
        return BADSAVEFILE;
    }
    if (!read_board(&game->board, input, width, height)) {
        fclose(input);
        return BADSAVEFILE;
    }
    game->emptyBoard = board_empty(&game->board);
    game->current = next=='1'?0:1;
    game->deck.used = drawn;
    fclose(input);
    return OK;
}

// might be cleaner to pass a flag for different behaviour
void print_hand(FILE* output, Hand* hand, bool spaced)
{
    for (unsigned i = 0; i < hand->size; ++i) {
        fputc(hand->cards[i].rank, output);
        fputc(hand->cards[i].suit, output);
        if (spaced && (i < hand->size - 1)) {
            fputc(' ', output);
        }
    }
    fputc('\n', output);
}

// Send board to specified FILE*
// bool parameter determines whether to use the stdout blank char
// or the save file blank char
void print_grid(FILE* output, Board* board, bool forStdout)
{
    char empty = (forStdout ? BLANKCHAR : BLANKFILECHAR);
    for (unsigned i = 0; i < board->height; ++i) {
        for (unsigned j = 0; j < board->width; ++j) {
            if (board->grid[i][j].rank == BLANKCHAR) {
                fputc(empty, output);
                fputc(empty, output);
            } else {
                fputc(board->grid[i][j].rank, output);
                fputc(board->grid[i][j].suit, output);
            }
        }
        fputc('\n', output);
    }
}

// is the specified cell within the bounds of the board
bool in_bounds(Board* board, unsigned short c, unsigned short r)
{
    return (c < board->width) && (r < board->height);
}

// is a particular cell empty
bool is_empty(Board* board, unsigned short c, unsigned short r)
{
    return board->grid[r][c].rank == BLANKCHAR;
}

// Save game state to file named fname
void save_game(Game* g, const char* fname)
{
    const char* message = "Unable to save\n";
    if (fname[0] == '\0') {
        fputs(message, stdout);
        return;
    }
    FILE* out = fopen(fname, "w");
    if (out) {
        fprintf(out, "%u %u %u %hu\n", g->board.width, g->board.height,
                g->deck.used, g->current + 1);
        fputs(g->deckname, out);
        fputc('\n', out);
        print_hand(out, &g->hand[0], false);
        print_hand(out, &g->hand[1], false);
        print_grid(out, &g->board, false);
        fclose(out);
    } else {
        fputs(message, stdout);
    }
}

// remove the card at the specified position in the current player's hand
// and place it in the specified cell
void play_card(Game* game, unsigned short cardpos, unsigned short column,
        unsigned row)
{
    if ((cardpos > MAXHANDSIZE) || !in_bounds(&game->board, column, row)) {
        return;
    }
    Card c = game->hand[game->current].cards[cardpos];
    game->board.grid[row][column] = c;
    // now we need to move cards down
    for (int i = cardpos; i < MAXHANDSIZE - 1; ++i) {
        game->hand[game->current].cards[i] =
                game->hand[game->current].cards[i + 1];
    }
    game->hand[game->current].size--;
}

// Add a new card in the last position in the current player's
// hand.  Assumes that there are exactly MAXHANDSIZE-1 cards in
// the hand when this function is called
void add_card(Game* game, Card card)
{
    game->hand[game->current].cards[MAXHANDSIZE - 1] = card;
    game->hand[game->current].size = MAXHANDSIZE;
}

// Does the specified cell have a neighbouring cell which is not empty
bool has_neighbour(Board* board, unsigned short column, unsigned short row)
{
    if (!in_bounds(board, column, row)) {
        return false;
    }
    unsigned short width = board->width;
    unsigned short height = board->height;
    if (!is_empty(board, (column + width - 1) % width, row)) {  // West
        return true;
    }
    if (!is_empty(board, (column + 1) % width, row)) {  // East
        return true;
    }
    if (!is_empty(board, column, (row + 1) % height)) { // South
        return true;
    }
    if (!is_empty(board, column, (row + height - 1) % height)) { // North
        return true;
    }
    return false;
}

// Prompt the user to input a move for the current player
// then play that card
// Return OK status on success, other status otherwise
Status get_human_move(Game* game)
{
    char buffer[80]; // spec should say that no valid move is longer than this
    printf("Hand(%d): ", game->current ? 2 : 1);
    print_hand(stdout, &game->hand[game->current], true);
    while (true) {
        fputs("Move? ", stdout);
        if (!fgets(buffer, 80, stdin)) {
            return EOINPUT;
        }
        if (buffer[strlen(buffer) - 1] != '\n') {       // long line
            int c;
            while (c = fgetc(stdin), c != '\n') {
                if (c == EOF) {
                    return EOINPUT;
                }
            }
        } else {
            unsigned short card, c, r;
            char dummy;
            if (!strncmp("SAVE", buffer, 4)) {
                if (buffer[strlen(buffer) - 1] == '\n') {
                    buffer[strlen(buffer) - 1] = '\0';
                }
                save_game(game, buffer + 4);
            } else if (sscanf(buffer, "%hu %hu %hu%c", &card, &c, &r, 
                    &dummy) == 4 && dummy == '\n') {  
                // doesn't check for extra spaces
                if ((card == 0) || (card > 6) 
                        || !in_bounds(&game->board, c - 1, r - 1)
                        || !is_empty(&game->board, c - 1, r - 1)
                        || (!game->emptyBoard && 
                        !has_neighbour(&game->board, c - 1, r - 1))) {
                    continue;
                }
                play_card(game, card - 1, c - 1, r - 1);
                break;
            }
        }
    }
    return OK;
}

// Get a move from an automated player and add it to the board
void get_a_move(Game* game)
{
    fputs("Hand: ", stdout);
    print_hand(stdout, &game->hand[game->current], true);
    if (game->current == 0) {   // player1
        for (unsigned short i = 0; i < game->board.height; ++i) {
            for (unsigned short j = 0; j < game->board.width; ++j) {
                if (is_empty(&game->board, j, i)
                        && has_neighbour(&game->board, j, i)) {
                    Card c = game->hand[0].cards[0];
                    printf("Player 1 plays %c%c in column %hu row %hu\n",
                            c.rank, c.suit, j + 1, i + 1);
                    play_card(game, 0, j, i);
                    return;
                }
            }
        }
        // if we get here, then the board was empty
        unsigned short row = (game->board.height - 1) / 2;
        unsigned short col = (game->board.width - 1) / 2;
        Card c = game->hand[0].cards[0];
        printf("Player 1 plays %c%c in column %hu row %hu\n",
                c.rank, c.suit, col + 1, row + 1);
        play_card(game, 0, col, row);
    } else {
        for (unsigned short i = 0; i < game->board.height; ++i) {
            for (unsigned short j = 0; j < game->board.width; ++j) {
                unsigned short row = game->board.height - 1 - i;
                unsigned short col = game->board.width - 1 - j;
                if (is_empty(&game->board, col, row)
                        && has_neighbour(&game->board, col, row)) {
                    Card c = game->hand[1].cards[0];
                    printf("Player 2 plays %c%c in column %hu row %hu\n",
                            c.rank, c.suit, col + 1, row + 1);
                    play_card(game, 0, col, row);
                    return;
                }
            }
        }
    }
}

// is the card at the specified location of higher rank
// than the rank parameter
// True if cell is empty or contains a higher ranked card
bool is_greater(Board* board, char rank, unsigned short col,
        unsigned short row)
{
    return (board->grid[row][col].rank != BLANKCHAR)
            && (board->grid[row][col].rank > rank);
}

// calculate the maximum points possible for a path travelling
// through the specified cell.
// The path must use the specified suit
unsigned score_position(Board* board, unsigned short column,
        unsigned short row, char suit)
{
    unsigned short width = board->width;
    unsigned short height = board->height;
    Card current = board->grid[row][column];
    unsigned newTotal = (current.suit == suit ? 1 : 0);
    // West
    if (is_greater(board, current.rank, (column + width - 1) % width, row)) {
        unsigned result =
                score_position(board, (column + width - 1) % width, row, suit);
        if ((result != 0) && (result + 1 > newTotal)) {
                                // we have found a later end point
            newTotal = result + 1;      // and this path is longer
        }
    }
    // East
    if (is_greater(board, current.rank, (column + 1) % width, row)) {
        unsigned result =
                score_position(board, (column + 1) % width, row, suit);
        if ((result != 0) && (result + 1 > newTotal)) { 
                                        // we have found a later end point
            newTotal = result + 1;      // and this path is longer
        }
    }
    // South
    if (is_greater(board, current.rank, column, (row + 1) % height)) {
        unsigned result =
                score_position(board, column, (row + 1) % height, suit);
        if ((result != 0) && (result + 1 > newTotal)) { 
                                        // we have found a later end point
            newTotal = result + 1;      // and this path is longer
        }
    }
    // North
    if (is_greater(board, current.rank, column, (row + height - 1) % height)) {
        unsigned result =
                score_position(board, column, (row + height - 1) % height, 
                suit);
        if ((result != 0) && (result + 1 > newTotal)) { 
                                        // we have found a later end point
            newTotal = result + 1;      // and this path is longer
        }
    }
    return newTotal;
}

// Calculate scores for both players
void do_scores(Board* board)
{
    unsigned totals[2];
    totals[0] = totals[1] = 0;
    for (unsigned short r = 0; r < board->height; ++r) {
        for (unsigned short c = 0; c < board->width; ++c) {
            if (!is_empty(board, c, r)) {
                char suit = board->grid[r][c].suit;
                unsigned tot = score_position(board, c, r, suit);
                if (totals[(suit - 'A') % 2] < tot) {
                    totals[(suit - 'A') % 2] = tot;
                }
            }
        }
    }
    printf("Player 1=%u Player 2=%u\n", totals[0], totals[1]);
}

// returns OK on normal game over
// if loaded is true, then don't draw a new card on the first turn
Status run_game(Game* game, bool loaded)
{
    print_grid(stdout, &game->board, true);
    Card c;
        // what if they load a full board?
    while (loaded || draw_card(&game->deck, &c)) {
        if (!loaded) {
            add_card(game, c);
        }
        loaded = false;        
        int current = game->current;
        char type = game->types[current];
        if (type == 'h') {
            Status result = get_human_move(game);
            if (result != OK) {
                return result;
            }
            print_grid(stdout, &game->board, true);
        } else if (type == 'a') {
            get_a_move(game);
            print_grid(stdout, &game->board, true);
        }
        game->emptyBoard = false;
        game->current = (game->current + 1) % 2;
        if (game->deck.used - 2 * (MAXHANDSIZE - 1) >=
                game->board.width * game->board.height) {
            break;              // board is full
        }
    }
    do_scores(&game->board);
    return OK;
}

// set up the game structure (except for the deck)
void init_game(Game* game, unsigned width, unsigned height,
        char ptype1, char ptype2)
{
    game->types[0] = ptype1;
    game->types[1] = ptype2;
    init_board(&game->board, width, height);
    game->hand[0].size = 0;
    game->hand[1].size = 0;
    game->current = 0;
}

// Give each player their hand of cards
// True if there are enough cards, false otherwise
bool draw_hands(Game* game)
{
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < MAXHANDSIZE - 1; ++j) {
            Card c;
            if (!draw_card(&game->deck, &c)) {
                return false;
            }
            game->hand[i].cards[j] = c;
        }
        game->hand[i].size = MAXHANDSIZE - 1;
    }
    if (game->deck.used == game->deck.count) {  // not enough to draw 1st card
        return false;
    }
    return true;
}

// Start a new game with an empty board
// return OK on success, error status otherwise
// don't call this function unless argv has enough entries
Status new_game(char** argv) {
    char ptype[2];
    unsigned int width, height;
    Game game;
    game.emptyBoard = true;
    for (int i = 0; i < 2; ++i) {
        if (!strcmp(argv[4 + i], "a")) {
            ptype[i] = 'a';
        } else if (!strcmp(argv[4 + i], "h")) {
            ptype[i] = 'h';
        } else {
            return show_message(BADARGTYPE);
        }
    }
    char* err1, *err2;
    width = strtoul(argv[2], &err1, 10);
    height = strtoul(argv[3], &err2, 10);
    if (*err1 != '\0' || *err2 != '\0' || !valid_dim(height) 
            || !valid_dim(width)) {
        return show_message(BADARGTYPE);
    }
    if (!load_deck_file(argv[1], &game.deck)) {
        return show_message(BADDECKFILE);
    }
    game.deckname = malloc(sizeof(char) * (strlen(argv[1] + 1)));
    strcpy(game.deckname, argv[1]);
    game.types[0] = ptype[0];
    game.types[1] = ptype[1];
    init_game(&game, width, height, ptype[0], ptype[1]);
    if (!draw_hands(&game)) {
        return show_message(SHORTDECK);
    }    
    return show_message(run_game(&game, false));
}

// load the state of the game from a file
// return OK on success, error status otherwise
// do not call this function unless argv has enough entries in it
Status load_game(char** argv) {
    char ptype[2];
    Game game;
    for (int i = 0; i < 2; ++i) {
        if (!strcmp(argv[2 + i], "a")) {
            ptype[i] = 'a';
        } else if (!strcmp(argv[2 + i], "h")) {
            ptype[i] = 'h';
        } else {
            return show_message(BADARGTYPE);
        }
    }
    game.emptyBoard = true;
    Status result = load_saved(argv[1], &game);
    if (result != OK) {
        return show_message(result);
    }
    game.types[0] = ptype[0];
    game.types[1] = ptype[1];   
    return show_message(run_game(&game, true));
}

int main(int argc, char** argv)
{
    if (argc == 6) {
        return new_game(argv);
    } else if (argc == 4) {
        return load_game(argv);
    } else {
        return show_message(BADARGNUM);
    }
}
