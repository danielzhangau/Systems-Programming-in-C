#ifndef SHARED_H_
#define SHARED_H_
#include <stdbool.h>

#define BUFFER_SIZE 150 
// largest message no more than 150 (including \n and EOF)

typedef struct CardListNode CardListNode;
typedef struct PathListNode PathListNode;
typedef struct PlayerListNode PlayerListNode;

struct PlayerListNode {
    int pid;
    struct PlayerListNode* before;
    struct PlayerListNode* after;
};

typedef struct {
    PlayerListNode* firstNode;
    PlayerListNode* lastNode;
    int size;   // can be known as current player number inside site
} PlayerList;

typedef struct {
    char type[2];
    int capacity;
    int siteIndex;
    PlayerList* playerList;
} Site;

struct CardListNode {
    char card;
    struct CardListNode* before;
    struct CardListNode* after;
};

typedef struct {
    CardListNode* firstNode;
    CardListNode* lastNode;
    CardListNode* current;
    int size;
} CardList;

struct PathListNode {
    Site* site;
    struct PathListNode* before;
    struct PathListNode* after;
};

typedef struct {
    PathListNode* firstNode;
    PathListNode* lastNode;
    int size;
} PathList;

void list_add_card(CardList* cardList, int cardIndex);

void path_add_site(PathList* pathList, Site* newSite);

void player_add_id(PlayerList* playerList, int pid);

char deck_get_card(CardList* cardList);

void list_remove_player(PlayerList* playerList);

CardList* list_create();

PathList* path_create();

PlayerList* player_create();

bool valid_card(char suite);

bool valid_site(char* type, int capacity);

#endif