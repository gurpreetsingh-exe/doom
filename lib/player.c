#include "player.h"
#include <stdlib.h>

Player* player_init(Thing thing) {
  Player* player = (Player*)malloc(sizeof(Player));
  player->pos = (Vec2){.x = thing.x, .y = thing.y};
  player->angle = RADIANS(thing.angle);
  return player;
}

bool player_is_on_side(Player* player, Node* node) {
  int dx = player->pos.x - node->x_partition;
  int dy = player->pos.y - node->y_partition;
  return dx * node->dy_partition - dy * node->dx_partition <= 0;
}

void player_destroy(Player* player) { free(player); }
