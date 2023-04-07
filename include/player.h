#ifndef PLAYER_H
#define PLAYER_H

#include "vec2.h"
#include "wad.h"

typedef struct {
  Vec2 pos;
  float angle;
} Player;

Player* player_init(Thing thing);
bool player_is_on_side(Player* player, Node* node);
void player_destroy(Player* player);

#endif // !PLAYER_H
