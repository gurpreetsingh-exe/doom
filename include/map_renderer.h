#ifndef MAP_RENDERER_H
#define MAP_RENDERER_H

#include "map.h"
#include "player.h"
#include "renderer.h"

typedef struct {
  DoomMap* map;
  Renderer* renderer;
  Player* player;
} MapRenderer;

MapRenderer* map_renderer_init(DoomMap* map, Renderer* renderer,
                               Player* player);
bool map_renderer_check_bbox(MapRenderer* map_renderer, BBox bbox);
bool map_renderer_add_segment(MapRenderer* map_renderer, Vec2 v0, Vec2 v1,
                              int* x1, int* x2);
void map_renderer_draw_segment(MapRenderer* map_renderer, Segment seg);
void map_renderer_draw_vlines(MapRenderer* map_renderer, Segment seg, int x1,
                              int x2);
void map_renderer_draw_map(MapRenderer* map_renderer);
void map_renderer_draw_node(MapRenderer* map_renderer, int16_t node_id);
void map_renderer_draw_subsector(MapRenderer* map_renderer, int16_t node_id);
void map_renderer_draw_bsp_node(MapRenderer* map_renderer, int16_t node_id);
void map_renderer_destroy(MapRenderer* map_renderer);

#endif // !MAP_RENDERER_H
