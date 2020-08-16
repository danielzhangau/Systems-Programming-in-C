#ifndef PLAYER_STRATEGY_H_
#define PLAYER_STRATEGY_H_
#include <stdbool.h>
#include "player.h"
#include "shared.h"

bool has_most_card(Game* game, int playerId);

bool all_zero_card(Game* game);

bool all_node_later(Game* game, PathListNode* currentNode);

int find_next_barrier(Game* game, PathListNode* currentNode);

PathListNode find_current_index(Game* game);

void player_decide_move(Game* game);

#endif