#ifndef MAP_RENDERER_H
#define MAP_RENDERER_H

#include "config.h"
#include "map.h"
#include "player.h"
#include "renderer.h"

typedef struct {
  DoomMap* map;
  Renderer* renderer;
  Player* player;
  Config* config;
} MapRenderer;

MapRenderer* map_renderer_init(DoomMap* map, Renderer* renderer, Player* player,
                               Config* config);
bool map_renderer_check_bbox(MapRenderer* map_renderer, BBox bbox);
void map_renderer_draw_map(MapRenderer* map_renderer);
void map_renderer_draw_node(MapRenderer* map_renderer, int16_t node_id);
void map_renderer_draw_subsector(MapRenderer* map_renderer, int16_t node_id);
void map_renderer_draw_bsp_node(MapRenderer* map_renderer, int16_t node_id);
void map_renderer_destroy(MapRenderer* map_renderer);

#endif // !MAP_RENDERER_H
