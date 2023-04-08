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
  return true;
#if 0
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
    float span1 = norm_angle(a0 + HALF_FOV);
    if (span1 > FOV) {
      if (span1 >= span + FOV) {
        continue;
      }
      return true;
    }
  }
  return false;
#endif
}

bool map_renderer_add_segment(MapRenderer* map_renderer, Vec2 v0, Vec2 v1,
                              int* x1, int* x2) {
  Player* player = map_renderer->player;
  float a0 = player_angle_to_vec(player, v0);
  float a1 = player_angle_to_vec(player, v1);
  float span = norm_angle(a0 - a1);
  if (span >= 180) {
    return false;
  }

  a0 = a0 - player->angle;
  a1 = a1 - player->angle;

  float span1 = norm_angle(a0 + HALF_FOV);
  if (span1 > FOV) {
    if (span1 >= span + FOV) {
      return false;
    }
    a0 = HALF_FOV;
  }

  float span2 = norm_angle(HALF_FOV - a1);
  if (span2 > FOV) {
    if (span2 >= span + FOV) {
      return false;
    }
    a1 = -HALF_FOV;
  }

  int hw = (map_renderer->renderer->image->width - 1) / 2;

  *x1 = angle_to_screen(a0, hw);
  *x2 = angle_to_screen(a1, hw);

  return true;
}

void map_renderer_draw_map(MapRenderer* map_renderer) {
  DoomMap* map = map_renderer->map;
  if (config.top_view && config.display_map) {
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

void map_renderer_draw_vlines(MapRenderer* map_renderer, Segment seg, int x1,
                              int x2) {
  srand(seg.start_vertex + seg.end_vertex);
  uint8_t r = rand() % 255;
  uint8_t g = rand() % 255;
  uint8_t b = rand() % 255;
  uint8_t a = 255;
  uint32_t color = (a << 24) | (b << 16) | (g << 8) | r;
  Image* image = map_renderer->renderer->image;
  renderer_draw_line(map_renderer->renderer, vec2(x1, 0),
                     vec2(x1, image->height - 1), color);
  renderer_draw_line(map_renderer->renderer, vec2(x2, 0),
                     vec2(x2, image->height - 1), color);
}

void map_renderer_draw_subsector(MapRenderer* map_renderer, int16_t node_id) {
  DoomMap* map = map_renderer->map;
  SubSector sub_sector = map->subsectors[node_id];
  for (size_t i = 0; i < sub_sector.seg_count; ++i) {
    Segment seg = map->segments[sub_sector.first_seg + i];
    Vec2 v1 = map->vertices[seg.start_vertex];
    Vec2 v2 = map->vertices[seg.end_vertex];
    int x = 0, y = 0;
    if (config.top_view) {
      if (config.clip_view) {
        if (map_renderer_add_segment(map_renderer, v1, v2, &x, &y)) {
          config.segments += 1;
          map_renderer_draw_segment(map_renderer, seg);
        }
      } else {
        config.segments += 1;
        map_renderer_draw_segment(map_renderer, seg);
      }
    } else {
      if (map_renderer_add_segment(map_renderer, v1, v2, &x, &y)) {
        config.segments += 1;
        map_renderer_draw_vlines(map_renderer, seg, x, y);
      }
    }
  }
}

void map_renderer_draw_segment(MapRenderer* map_renderer, Segment seg) {
  DoomMap* map = map_renderer->map;
  Vec2 v1 = map->vertices[seg.start_vertex];
  Vec2 v2 = map->vertices[seg.end_vertex];
  srand(seg.start_vertex + seg.end_vertex);
  v1 = vec2_remap_window(v1, map->min_pos, map->max_pos);
  v2 = vec2_remap_window(v2, map->min_pos, map->max_pos);
  uint8_t r = rand() % 255;
  uint8_t g = rand() % 255;
  uint8_t b = rand() % 255;
  uint8_t a = 255;
  uint32_t color = (a << 24) | (b << 16) | (g << 8) | r;
  renderer_draw_line(map_renderer->renderer, v1, v2, color);
}

void map_renderer_draw_bsp_node(MapRenderer* map_renderer, int16_t node_id) {
  if (node_id & SSECTOR_IDENTIFIER) {
    if (config.debug_sectors) {
      map_renderer_draw_subsector(map_renderer,
                                  node_id & (~SSECTOR_IDENTIFIER));
    }
    return;
  }

  if (config.top_view && config.debug_bsp_view) {
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
