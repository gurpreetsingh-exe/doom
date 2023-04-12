#ifndef PLAYER_H
#define PLAYER_H

#include "vec2.h"
#include "wad.h"

typedef struct {
  Vec2 pos;
  float angle;
  int z;
} Player;

Player* player_init(Thing thing);
bool player_is_on_side(Player* player, Node* node);
float player_angle_to_vec(Player* player, Vec2 v);
float player_distance_from_point(Player* player, Vec2 p);
void player_destroy(Player* player);

#endif // !PLAYER_H
