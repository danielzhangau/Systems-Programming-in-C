#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <signal.h>
#include <sys/wait.h> 
#include "shared.h"

#define ARG_STRING_SIZE 10
#define PIPE_READ 0
#define PIPE_WRITE 1

#define BLANK_CHAR ' '

/** An enum
 * Define exit status 
 */
typedef enum {
    NORMAL_GAME_OVER = 0,
    WRONG_ARG_NUMBER = 1,
    DECK_READ_FAIL = 2,
    PATH_READ_FAIL = 3,
    UNABLE_TO_START_PLAYER = 4,
    COMMUNICATION_ERROR = 5
} Status;

/** Output error message for status and return status
 *	- Returns nothing
 *	- Prints exit status messages out to stderr(Standard error) 
 */
Status exit_message(Status s) {
    const char* messages[] = {"",                    //0
            "Usage: 2310dealer deck path p1 {p2}\n", //1
            "Error reading deck\n",                  //2
            "Error reading path\n",                  //3
            "Error starting process\n",              //4
            "Communications error\n"};               //5
    fputs(messages[s], stderr);
    return s;
}

typedef struct {
    int pid;
    char space1;
    char space2;
} Cell;

typedef struct {
    unsigned int height, width;
    Cell** grid;
} Board;

typedef struct {
    FILE* input;
    FILE* output;
    int pid;
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
    CardList* deck;
    PathList* path;
    unsigned short numberOfPlayers;
    Player** players;
    Board board;
} Game;

// reference to the game struct to be used by signal handler
Game* sigGame;

/**
 * @brief  loads the deck from file at path game->deckfile. 
 * @note   requires that game->deckfile is set. Execution may result in process
 *      terminating with DECK_READ_FAIL
 * @param  game: the game struct
 * @retval None
 */
Status load_deck(Game* game, FILE* deckFile) {
    char buffer[BUFFER_SIZE];
    // Get the whole deck line e.g. 7ABACDEE
    if(!fgets(buffer, BUFFER_SIZE, deckFile)) {
        return DECK_READ_FAIL;
    }
    unsigned int numberOfCards;
    if ((sscanf(buffer, "%u", &numberOfCards) != 1) || (numberOfCards < 4)) {
        return DECK_READ_FAIL;     
    }
    char* restBuffer = buffer + 1;
    if (numberOfCards >= 10) {
        restBuffer = restBuffer + 1;
    }

    // Create deck list and populate
    game->deck = list_create();
    for (unsigned int i = 0; i < numberOfCards; i++) {
        char suite;
        if ((sscanf(restBuffer, "%c", &suite) != 1) || !valid_card(suite)) {
            return DECK_READ_FAIL;    
        }
        restBuffer = restBuffer + 1;
        int newCardIndex;
        if (suite == 'A') {
            newCardIndex = 1;
        } else if (suite == 'B') {
            newCardIndex = 2;
        } else if (suite == 'C') {
            newCardIndex = 3;
        } else if (suite == 'D') {
            newCardIndex = 4;
        } else if (suite == 'E') {
            newCardIndex = 5;
        }
        list_add_card(game->deck, newCardIndex);
    }
    if (restBuffer[0] != '\n') {
        return DECK_READ_FAIL;
    }
    if (fgets(restBuffer, BUFFER_SIZE, deckFile)) {
        // excess content in deck
        return DECK_READ_FAIL;
    }

    return NORMAL_GAME_OVER;
} 

/**
 * @brief  loads the path from file at path game->pathfile. 
 * @note   requires that game->pathfile is set. Execution may result in process
 *      terminating with PATH_READ_FAIL
 * @param  game: the game struct
 * @retval None
 */
Status load_path(Game* game, FILE* pathFile) {
    char buffer[BUFFER_SIZE], symbol, site[2], mi; // symbol: ';'   mi: '-' 
    if (!fgets(buffer, BUFFER_SIZE, pathFile)) { // Get the whole deck line
        return COMMUNICATION_ERROR;
    }
    unsigned int numOfSites, a;// a: the capacity of the site--use later
    if ((sscanf(buffer, "%u", &numOfSites) != 1 || numOfSites < 2) && 
            (sscanf(buffer, "%c", &symbol) != 1 || symbol != ';')) { 
        return PATH_READ_FAIL;    
    }
    char* restBuffer = buffer + 2;
    if (numOfSites >= 10) {
        restBuffer = restBuffer + 1;
        if (buffer[3 + numOfSites * 3] != '\n') { //check unexpected eof
            return PATH_READ_FAIL;
        }
    } else if ((numOfSites < 10) && (buffer[2 + numOfSites * 3] != '\n')) {
        return PATH_READ_FAIL;
    }

    game->path = path_create(); // Create Pathlist and populate
    for (unsigned int i = 0; i < numOfSites; i++) {  
        if (sscanf(restBuffer, "%2s", site) != 1) {
            return PATH_READ_FAIL;     
        }
        restBuffer = restBuffer + 2;
        if ((i == 0 || i == numOfSites - 1) && (strcmp(site, "::") != 0)) {
            return PATH_READ_FAIL;       // check first, last
        }
        if ((strcmp(site, "::") == 0) && 
                (sscanf(restBuffer, "%c", &mi) != 1 || mi != '-')) {
            return PATH_READ_FAIL;      
        } else if ((strcmp(site, "::") != 0) && (sscanf(restBuffer, 
                "%u", &a) != 1 || !valid_site(site, a))) {
            return PATH_READ_FAIL;
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
        path_add_site(game->path, newSite);
    }
    return NORMAL_GAME_OVER;
} 

/**
 * @brief  execs the process as one player with the required parameters
 * @note   execution of current executables is replaced if successfull, 
 *      otherwise program terminates with exit status 
 *      UNABLE_TO_START_PLAYER
 * @param  processPath: the path to process executable
 * @param  playerNumber: the player id
 * @retval None
 */
void exec_as_player_process(Game* game, const char* processPath, 
        int playerNumber) {
    // generate the arguments to pass when calling exec
    char players[ARG_STRING_SIZE];
    sprintf(players, "%d", game->numberOfPlayers);
    char myId[ARG_STRING_SIZE];
    sprintf(myId, "%d", playerNumber);

    // execute desired child process
    if(execl(processPath, "player", players, myId, NULL) < 0) {
        exit(UNABLE_TO_START_PLAYER);
    } 
    // execl(processPath, players, myId, NULL);
    // The  exec() functions return only if an error has occurred.  The return
    // value is -1, and errno is set to indicate the error.
}

/**
 * @brief  Starts a child process to act as a player. Addtionally 
 *      configures pipes so 2310dealer[fd-A] -> child[stdin] and 
 *      child[stdout] -> 2310[fd-B].
 * @note   can terminate the program with exit status 
 *      UNABLE_TO_START_PLAYER
 * @param  process_path: path to player executable
 * @retval newly created Player with input, output and pid set
 */
Player* start_player_process(Game* game, const char* processPath, 
        int playerNumber) {
    // create pipes
    int dealerToPlayer[2], playerToDealer[2];
    if (pipe(dealerToPlayer) || pipe(playerToDealer)) {
        exit_message(UNABLE_TO_START_PLAYER);
    }

    // create new player struct
    Player* player = (Player*)malloc(sizeof(Player));
    // fork and close un-needed file descriptors
    player->pid = fork();
    if (player->pid < 0) {    // error in fork
        exit_message(UNABLE_TO_START_PLAYER); 
    } else if (player->pid) {
        // parent
        player->input = fdopen(dealerToPlayer[PIPE_WRITE], "w"); // [1]
        player->output = fdopen(playerToDealer[PIPE_READ], "r"); // [0]
        player->points = 0;
        player->money = 7;
        player->hand = list_create();
        player->countV1 = 0;
        player->countV2 = 0;
        player->countA = 0;
        player->countB = 0;
        player->countC = 0;
        player->countD = 0;
        player->countE = 0;

        // close un-needed file descriptors
        close(dealerToPlayer[PIPE_READ]);
        close(playerToDealer[PIPE_WRITE]);
    } else {
        // child
        // parent [write(1)] -> child [gcread(0)]
        dup2(dealerToPlayer[PIPE_READ], STDIN_FILENO); // 0
        close(dealerToPlayer[PIPE_WRITE]);

        // child [write(1)] -> parent [read(0)]
        dup2(playerToDealer[PIPE_WRITE], STDOUT_FILENO); // 1
        close(playerToDealer[PIPE_READ]);

        // suppress stderr
        freopen("/dev/null", "w", stderr);
        
        exec_as_player_process(game, processPath, playerNumber);
    }
    return player;
}

/**
 * @brief  send a YT message. player need to get back a DO move
 * @param  player: player to be sended
 * @retval none
 */
void send_message_yt(Player* player) {
    fprintf(player->input, "YT\n");
    fflush(player->input);
    return;
}

/**
 * @brief  send a HAP message base on given parameters
 * @param  player: player to be sended
 * @param  pid - newCard: all updated player info
 * @retval none
 */
void send_message_hap(Player* player, int pid, int newSite, int addedPoints, 
        int addedMoney, int newCard) {
    fprintf(player->input, "HAP%d,%d,%d,%d,%d\n", pid, newSite, 
            addedPoints, addedMoney, newCard); 
    fflush(player->input);
    return;
}

/**
 * @brief  sends player message indicating the game is over
 * @param  player: the player will be sent to 
 * @retval None
 */
void send_message_done(Player* player) {
    fprintf(player->input, "DONE\n");
    fflush(player->input);
    return;
}

/**
 * @brief  sends player message indicating the game ended earlier
 * @param  player: the player will be sent to 
 * @retval None
 */
void send_message_early(Player* player) {
    fprintf(player->input, "EARLY\n");
    fflush(player->input);
    return;
}

/**
 * @brief  print game path in format
 * @param  output: stderr or stdout
 * @param  game: current game
 * @retval none
 */
void print_path(FILE* output, Game* game) {
    PathListNode* nextNode = game->path->firstNode;
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
    PathListNode* nextNode = game->path->firstNode;
    int greatestPlayerListSize = 0;
    for (int i = 0; i < game->path->size; i++) {
        if (nextNode->site->playerList->size > greatestPlayerListSize) {
            greatestPlayerListSize = nextNode->site->playerList->size;
        }
        nextNode = nextNode->after;    
    }
    // initial board
    Board* board = &game->board;
    board->height = greatestPlayerListSize;
    board->width = game->path->size;
    board->grid = malloc(sizeof(Cell*) * greatestPlayerListSize);
    for (unsigned i = 0; i < greatestPlayerListSize; ++i) {
        board->grid[i] = malloc(sizeof(Cell) * game->path->size);
        for (unsigned j = 0; j < game->path->size; ++j) {
            board->grid[i][j].pid = BLANK_CHAR;
            board->grid[i][j].space1 = BLANK_CHAR;
            board->grid[i][j].space2 = BLANK_CHAR;
        }
    }

    //add content
    int j = 0;
    PathListNode* nextNode2 = game->path->firstNode;
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
 * @brief  convert the card suite into index and add to player info count
 * @param  suite: card suite ABCDE
 * @param  newCardIndex: input was intialized number
 * @retval newCardIndex after convert
 */
int parse_card(Player* player, char suite, int newCardIndex) {
    if (suite == 'A') {
        newCardIndex = 1;
        player->countA += 1;
    } else if (suite == 'B') {
        newCardIndex = 2;
        player->countB += 1;
    } else if (suite == 'C') {
        newCardIndex = 3;
        player->countC += 1;
    } else if (suite == 'D') {
        newCardIndex = 4;
        player->countD += 1;
    } else if (suite == 'E') {
        newCardIndex = 5;
        player->countE += 1;
    }
    return newCardIndex;
}

/**
 * @brief  print each pathlist->playerlist in format
 * @param  output: stderr or stdout
 * @retval none
 */
void player_make_move(Game* game, int siteIndex, int playerId) {
    // we remove the id from current place
    PathListNode* nextNode = game->path->firstNode;
    for (int i = 0; i < game->path->size; i++) {
        if (nextNode->site->playerList->lastNode != NULL) {
            list_remove_player(nextNode->site->playerList);
            break;
        } else {
            nextNode = nextNode->after;
        }   
    }
    
    // then move to new site index
    PathListNode* nextNode2 = game->path->firstNode;
    PlayerList* playerList;
    for (int i = 0; i < game->path->size; i++) {
        if (nextNode2->site->siteIndex == siteIndex) {
            playerList = nextNode2->site->playerList;
            break;
        } else {
            nextNode2 = nextNode2->after;
        }
    }
    player_add_id(playerList, playerId);
    //update player info base on site type
    Player* player = game->players[playerId];
    int money = player->money;
    int addedMoney = 0, addedPoints = 0, newCardIndex = 0;
    if (strcmp(nextNode2->site->type, "V1") == 0) {
        player->countV1 += 1;
    } else if (strcmp(nextNode2->site->type, "V2") == 0) {
        player->countV2 += 1;
    } else if (strcmp(nextNode2->site->type, "Mo") == 0) {
        player->money += 3;
        addedMoney += 3;
    } else if (strcmp(nextNode2->site->type, "Do") == 0) {
        player->points += floor(money / 2);
        addedPoints -= player->points;
        player->money = 0;
        addedMoney = player->money - money;
    } else if (strcmp(nextNode2->site->type, "Ri") == 0) {
        char suite = deck_get_card(game->deck);
        newCardIndex = parse_card(player, suite, newCardIndex);
        list_add_card(player->hand, newCardIndex);
    }
    for (int i = 0; i < game->numberOfPlayers; i++) {
        send_message_hap(game->players[i], playerId, siteIndex, addedPoints, 
                addedMoney, newCardIndex);
    }
}

/**
 * @brief  parses a DO message. 
 * @note   can terminate with code COMMUNICATION_ERROR
 * @param  playerId: the player will update info
 * @param  message: DOn from player
 * @retval communication status
 */
Status parse_message_do(Game* game, char* message, int playerId) {
    if (isspace(message[0])) {
        // strtoul allows leading spaces
        return COMMUNICATION_ERROR;
    }
    // get newSite in message
    unsigned int newSite;
    if (sscanf(message, "%u", &newSite) != 1 || 
            newSite >= game->path->size || newSite < 0) {
        return COMMUNICATION_ERROR;
    }

    // playerId move to newsite number and update info
    player_make_move(game, newSite, playerId);

    // Print player information
    fprintf(stdout, "Player %d Money=%d V1=%d V2=%d "
            , playerId, game->players[playerId]->money, 
            game->players[playerId]->countV1, 
            game->players[playerId]->countV2);
    fprintf(stdout, "Points=%d A=%d B=%d C=%d D=%d E=%d\n", 
            game->players[playerId]->points, game->players[playerId]->countA, 
            game->players[playerId]->countB, game->players[playerId]->countC, 
            game->players[playerId]->countD, game->players[playerId]->countE);
    fflush(stdout);

    // reprint the game path and players
    print_path(stdout, game);
    print_player(stdout, game);

    return NORMAL_GAME_OVER;
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
        
        player->points = player->points + player->countV1;
        player->points = player->points + player->countV2;
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
 * @brief  prints each player's scores then sends a game over message 
 *      to all players. Then terminates the program with exit status 0
 * @retval None
 */
Status end_game(Game* game) {
    // print player scores
    calculate_score(game);
    print_score(stdout, game);
    fflush(stdout);

    // send gameover messages
    for (int i = 0; i < game->numberOfPlayers; i++) {
        send_message_done(game->players[i]);
    }

    return NORMAL_GAME_OVER;
}

/**
 * @brief  check if all the player are in the final site of pathlist
 * @retval true if yes, otherwise false
 */
bool all_in_last_site(Game* game) {
    if (game->path->lastNode->site->playerList->size 
            == game->numberOfPlayers) {
        return true;
    }
    return false;
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
    return NORMAL_GAME_OVER;
}

/**
 * @brief  runs the main loop of the game, parse message from player
 *      and make actions accordingly
 * @retval communication status
 */
Status start_game(Game* game) {
    print_path(stdout, game);
    print_player(stdout, game);

    // run loop until error or all player in last site
    while (!all_in_last_site(game)) {
        PathListNode* nextNode = game->path->firstNode;
        int currentId = 0;
        for (int i = 0; i < game->path->size; i++) {
            if (nextNode->site->playerList->lastNode != NULL) {
                currentId = nextNode->site->playerList->lastNode->pid;
                send_message_yt(game->players[currentId]);
                break;
            } else {
                nextNode = nextNode->after;
            }
        }

        char buffer[BUFFER_SIZE];
        if (!game->players[currentId]->output) {         // not open 
            return COMMUNICATION_ERROR; 
        }
        if(!fgets(buffer, BUFFER_SIZE, game->players[currentId]->output)) {
            return COMMUNICATION_ERROR; // unexpected EOF
        }
        // handle recieve message
        char* strAfterParse;
        if (!strncmp("DO", buffer, 2)) {
            Status status = parse_message_do(game, buffer + 2, currentId);
            if (status != NORMAL_GAME_OVER) {
                return status;
            }
            strAfterParse = buffer + 2; // DOn
            strAfterParse = strAfterParse + strlen(buffer + 2) - 1;
            // check remainder of message
            Status afterStatus = check_string_eof(strAfterParse);
            if (afterStatus != NORMAL_GAME_OVER) {
                return afterStatus;
            }
        } else {
            // unknown / malformed message header
            return COMMUNICATION_ERROR;
        }
    } 
    return exit_message(end_game(game));
}

/**
 * @brief  attempts to kill each child process with SIGTERM each reap them, if
 *      fail, kill the non compliant child with SIGKILL. Then terminates
 *      the program 
 * @note   should only handle SIGHUP signal as exits
 * @retval None
 */
void termination_handler(int signum) {
    // kill all process'
    for (int i = 0; i < sigGame->numberOfPlayers; i++) {
        if (sigGame->players[i]->pid == -1) {
            // process not yet created
            continue;
        }
        int pid = kill(pid, SIGTERM);

        // kill the child and reap it
        kill(pid, SIGTERM);
        bool died = false;
        for (int i = 0; i < 5; i++) {
            usleep(150 * 1000); // 150 msec
            if (waitpid(pid, NULL, WNOHANG) == pid) {
                died = true;
                break;
            }
        }
        if (!died) {
            kill(pid, SIGKILL);
        }
    }
}

int main(int argc, char const *argv[])
{
    if (argc < 4) {
        return exit_message(WRONG_ARG_NUMBER);
    }
    Game* game = (Game*)malloc(sizeof(Game));
    sigGame = game;
    // initalise game variables
    game->numberOfPlayers = argc - 3;
    game->players = (Player**)malloc(sizeof(Player) * game->numberOfPlayers);

    // load deck
    FILE* deckFile = fopen(argv[1], "r");
    if (!deckFile) {           // not open 
        return DECK_READ_FAIL; 
    }
    Status deckStatus = load_deck(game, deckFile);
    if (deckStatus != NORMAL_GAME_OVER) {
        fclose(deckFile); 
        return exit_message(deckStatus);
    }
    fclose(deckFile); 
    
    // Load path
    FILE* pathFile = fopen(argv[2], "r");
    if (!pathFile) {           // not open 
        return PATH_READ_FAIL; 
    }
    Status pathStatus = load_path(game, pathFile);
    if (pathStatus != NORMAL_GAME_OVER) {
        fclose(pathFile);
        return exit_message(pathStatus);
    }
    fclose(pathFile);

    // initalise signal handler
    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = termination_handler;   
    sa.sa_flags = SA_RESTART;  //Restart functions if interrupted by handler
    sigaction(SIGHUP, &sa, 0);

    // Start players
    for (int i = 0; i < game->numberOfPlayers; i++) {
        // Start new process for player
        Player* newPlayer = start_player_process(game, argv[i + 3], i);
        game->players[i] = newPlayer;

        // check that player started correctly
        int response = fgetc(newPlayer->output);
        if (response != '^') {
            return exit_message(UNABLE_TO_START_PLAYER);
        }
    }

    // add player into the game  FILO e.g 7 6 5 4 3 2 1<- last node
    for (int i = game->numberOfPlayers - 1; i >= 0; --i) {
        player_add_id(game->path->firstNode->site->playerList, i);
    }
    FILE* pathFileReopen = fopen(argv[2], "r");
    char pathString[BUFFER_SIZE];
    fgets(pathString, BUFFER_SIZE, pathFileReopen);
    for (int i = 0; i < game->numberOfPlayers; i++) {
        fprintf(game->players[i]->input, pathString);
        fflush(game->players[i]->input);
    }
    fclose(pathFileReopen);
    Status gameStatus = start_game(game);
    if (gameStatus != NORMAL_GAME_OVER) {
        for (int i = 0; i < game->numberOfPlayers; i++) {
            send_message_early(game->players[i]);
        }
        return exit_message(gameStatus);
    } else {
        return exit_message(NORMAL_GAME_OVER);
    }
}