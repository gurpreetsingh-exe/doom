#include "player.h"
#include "config.h"
#include <math.h>
#include <stdlib.h>

Player* player_init(Thing thing) {
  Player* player = (Player*)malloc(sizeof(Player));
  player->pos = (Vec2){.x = thing.x, .y = thing.y};
  player->angle = thing.angle;
  player->z = EYE_LEVEL;
  return player;
}

bool player_is_on_side(Player* player, Node* node) {
  int dx = player->pos.x - node->x_partition;
  int dy = player->pos.y - node->y_partition;
  return ((dx * node->dy_partition) - (dy * node->dx_partition)) <= 0;
}

float player_distance_from_point(Player* player, Vec2 p) {
  return sqrtf(powf(player->pos.x - p.x, 2) + powf(player->pos.y - p.y, 2));
}

float player_angle_to_vec(Player* player, Vec2 v) {
  // Vec2 dv = vec2_sub(v, player->pos);
  // return DEGREES(atan2f(dv.y, dv.x));
  return DEGREES(atan2f(v.y - player->pos.y, v.x - player->pos.x));
}

void player_destroy(Player* player) { free(player); }
