#include "map_renderer.h"
#include "config.h"
#include "player.h"
#include <stdlib.h>

extern Config config;

MapRenderer* map_renderer_init(DoomMap* map, Renderer* renderer,
                               Player* player) {
  MapRenderer* map_renderer = (MapRenderer*)malloc(sizeof(MapRenderer));
  map_renderer->map = map;
  map_renderer->renderer = renderer;
  map_renderer->player = player;
  return map_renderer;
}

bool map_renderer_check_bbox(MapRenderer* map_renderer, BBox bbox) {
  Vec2 a = vec2(bbox.left, bbox.bottom);
  Vec2 b = vec2(bbox.left, bbox.top);
  Vec2 c = vec2(bbox.right, bbox.top);
  Vec2 d = vec2(bbox.right, bbox.bottom);
  Player* player = map_renderer->player;
  Vec2 p = player->pos;
  Vec2 sides[4] = {0};
  int numsides = 0;

  if (p.x < bbox.left) {
    if (p.y > bbox.top) {
      sides[0] = b;
      sides[1] = a;
      sides[2] = c;
      sides[3] = b;
      numsides = 4;
    } else if (p.y < bbox.bottom) {
      sides[0] = b;
      sides[1] = a;
      sides[2] = a;
      sides[3] = d;
      numsides = 4;
    } else {
      sides[0] = b;
      sides[1] = a;
      numsides = 2;
    }
  } else if (p.x > bbox.right) {
    if (p.y > bbox.top) {
      sides[0] = c;
      sides[1] = b;
      sides[2] = d;
      sides[3] = c;
      numsides = 4;
    } else if (p.y < bbox.bottom) {
      sides[0] = a;
      sides[1] = d;
      sides[2] = d;
      sides[3] = c;
      numsides = 4;
    } else {
      sides[0] = d;
      sides[1] = c;
      numsides = 2;
    }
  } else {
    if (p.y > bbox.top) {
      sides[0] = c;
      sides[1] = b;
      numsides = 2;
    } else if (p.y < bbox.bottom) {
      sides[0] = a;
      sides[1] = d;
      numsides = 2;
    } else {
      return true;
    }
  }

  for (int i = 0; i < numsides; i += 2) {
    float a0 = player_angle_to_vec(player, sides[i]);
    float a1 = player_angle_to_vec(player, sides[i + 1]);
    int span = norm_angle(a0 - a1);
    a0 -= player->angle;
    int span1 = norm_angle(a0 + HALF_FOV);
    if (span1 > FOV) {
      if (span1 >= span + FOV) {
        continue;
      }
      return true;
    }
  }
  return false;
}

void map_renderer_draw_map(MapRenderer* map_renderer) {
  DoomMap* map = map_renderer->map;
  if (config.display_map) {
    for (size_t i = 0; i < map->numlinedefs; ++i) {
      LineDef linedef = map->linedefs[i];
      Vec2 v1 = map->vertices[linedef.start_vertex];
      Vec2 v2 = map->vertices[linedef.end_vertex];
      v1 = vec2_remap_window(v1, map->min_pos, map->max_pos);
      v2 = vec2_remap_window(v2, map->min_pos, map->max_pos);
      renderer_draw_line(map_renderer->renderer, v1, v2, 0xff222222);
    }
  }
  map_renderer_draw_bsp_node(map_renderer, map->numnodes - 1);
}

void map_renderer_draw_subsector(MapRenderer* map_renderer, int16_t node_id) {
  DoomMap* map = map_renderer->map;
  SubSector sub_sector = map->subsectors[node_id];
  for (size_t i = 0; i < sub_sector.seg_count; ++i) {
    Segment seg = map->segments[sub_sector.first_seg + i];
    srand(seg.start_vertex + seg.end_vertex);
    Vec2 v1 = map->vertices[seg.start_vertex];
    Vec2 v2 = map->vertices[seg.end_vertex];
    v1 = vec2_remap_window(v1, map->min_pos, map->max_pos);
    v2 = vec2_remap_window(v2, map->min_pos, map->max_pos);
    uint8_t r = rand() % 255;
    uint8_t g = rand() % 255;
    uint8_t b = rand() % 255;
    uint8_t a = 255;
    uint32_t color = (a << 24) | (b << 16) | (g << 8) | r;
    renderer_draw_line(map_renderer->renderer, v1, v2, color);
  }
}

void map_renderer_draw_bsp_node(MapRenderer* map_renderer, int16_t node_id) {
  if (node_id & SSECTOR_IDENTIFIER) {
    if (config.debug_sectors) {
      map_renderer_draw_subsector(map_renderer,
                                  node_id & (~SSECTOR_IDENTIFIER));
    }
    return;
  }

  if (config.debug_bsp_view) {
    map_renderer_draw_node(map_renderer, node_id);
  }
  Node* node = &map_renderer->map->nodes[node_id];
  if (player_is_on_side(map_renderer->player, node)) {
    map_renderer_draw_bsp_node(map_renderer, node->left_child);
    if (map_renderer_check_bbox(map_renderer, node->right)) {
      map_renderer_draw_bsp_node(map_renderer, node->right_child);
    }
  } else {
    map_renderer_draw_bsp_node(map_renderer, node->right_child);
    if (map_renderer_check_bbox(map_renderer, node->left)) {
      map_renderer_draw_bsp_node(map_renderer, node->left_child);
    }
  }
}

void map_renderer_draw_node(MapRenderer* map_renderer, int16_t node_id) {
  DoomMap* map = map_renderer->map;
  Node node = map_renderer->map->nodes[node_id];
  Vec2 v0 = vec2_remap_window(vec2(node.left.left, node.left.top), map->min_pos,
                              map->max_pos);
  Vec2 v1 = vec2_remap_window(vec2(node.left.right, node.left.bottom),
                              map->min_pos, map->max_pos);
  renderer_draw_rect(map_renderer->renderer, v0, v1, 0xff0000ff);

  v0 = vec2_remap_window(vec2(node.right.left, node.right.top), map->min_pos,
                         map->max_pos);
  v1 = vec2_remap_window(vec2(node.right.right, node.right.bottom),
                         map->min_pos, map->max_pos);
  renderer_draw_rect(map_renderer->renderer, v0, v1, 0xff00ff00);
}

void map_renderer_destroy(MapRenderer* map_renderer) { free(map_renderer); }
