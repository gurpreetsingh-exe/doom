#include "player.h"
#include <stdlib.h>

Player* player_init(Thing thing) {
  Player* player = (Player*)malloc(sizeof(Player));
  player->pos = (Vec2){.x = thing.x, .y = thing.y};
  player->angle = thing.angle;
  return player;
}

void player_destroy(Player* player) { free(player); }
