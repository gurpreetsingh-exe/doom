#include "map_renderer.h"
#include "config.h"
#include "player.h"
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

extern Config config;

static int hash(char name[8]) {
  int hash = 593049;
  for (int i = 0; i < 7; ++i) {
    hash += (hash << 5) + name[i];
  }
  return hash;
}

MapRenderer* map_renderer_init(DoomMap* map, Renderer* renderer,
                               Player* player) {
  MapRenderer* map_renderer = (MapRenderer*)malloc(sizeof(MapRenderer));
  map_renderer->map = map;
  map_renderer->renderer = renderer;
  map_renderer->player = player;
  return map_renderer;
}

static int16_t bbox_side_from_index(BBox bbox, int16_t index) {
  switch (index) {
  case 0:
    return bbox.top;
  case 1:
    return bbox.bottom;
  case 2:
    return bbox.left;
  case 3:
    return bbox.right;
  default:
    assert(false && "out of bounds index\n");
  }
}

int checkcoord[12][4] = {{3, 0, 2, 1}, {3, 0, 2, 0}, {3, 1, 2, 0}, {0},
                         {2, 0, 2, 1}, {0, 0, 0, 0}, {3, 1, 3, 0}, {0},
                         {2, 0, 3, 1}, {2, 1, 3, 1}, {2, 1, 3, 0}};

bool map_renderer_check_bbox(MapRenderer* map_renderer, BBox bbox) {
  Player* player = map_renderer->player;
  Vec2 p = player->pos;
  int16_t boxx, boxy;
  int16_t boxpos;
  if (p.x <= bbox.left) {
    boxx = 0;
  } else if (p.x < bbox.right) {
    boxx = 1;
  } else {
    boxx = 2;
  }

  if (p.y <= bbox.top) {
    boxy = 0;
  } else if (p.y < bbox.bottom) {
    boxy = 1;
  } else {
    boxy = 2;
  }

  boxpos = (boxy << 2) + boxx;
  if (boxpos == 5) {
    return true;
  }

  Vec2 p0 = vec2(bbox_side_from_index(bbox, checkcoord[boxpos][0]),
                 bbox_side_from_index(bbox, checkcoord[boxpos][1]));
  Vec2 p1 = vec2(bbox_side_from_index(bbox, checkcoord[boxpos][2]),
                 bbox_side_from_index(bbox, checkcoord[boxpos][3]));

  float a0 = player_angle_to_vec(player, p0) - player->angle;
  float a1 = player_angle_to_vec(player, p1) - player->angle;

  float span = norm_angle(a0 - a1);
  if (span >= 180) {
    return true;
  }

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

  int x0 = angle_to_screen(a0);
  int x1 = angle_to_screen(a1);

  if (x0 == x1) {
    return false;
  }

  return true;
}

bool map_renderer_add_segment(MapRenderer* map_renderer, Vec2 v0, Vec2 v1,
                              int* x1, int* x2, float* a0, float* a1) {
  Player* player = map_renderer->player;
  *a0 = player_angle_to_vec(player, v0);
  *a1 = player_angle_to_vec(player, v1);
  float span = norm_angle(*a0 - *a1);
  if (span >= 180) {
    return false;
  }

  *a0 = *a0 - player->angle;
  *a1 = *a1 - player->angle;

  float span1 = norm_angle(*a0 + HALF_FOV);
  if (span1 > FOV) {
    if (span1 >= span + FOV) {
      return false;
    }
    *a0 = HALF_FOV;
  }

  float span2 = norm_angle(HALF_FOV - *a1);
  if (span2 > FOV) {
    if (span2 >= span + FOV) {
      return false;
    }
    *a1 = -HALF_FOV;
  }

  *x1 = angle_to_screen(*a0);
  *x2 = angle_to_screen(*a1);

  if (*x1 == *x2) {
    return false;
  }

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

#if 0
void clip_solid_wall(MapRenderer* map_renderer, Segment* segment, int x1,
                     int x2, float a0, float a1) {
#if 1
  ClipRange* start;
  start = map_renderer->solidsegs;
  while (start->last < x1 - 1) {
    start++;
  }
  if (x1 < start->first) {
    if (x2 < start->first - 1) {
      store_wall_range(map_renderer, segment, x1, x2, a0, a1);
      return;
    }
    store_wall_range(map_renderer, segment, x1, start->first - 1, a0, a1);
  }
  if (x2 <= start->last) {
    return;
  }
  while (x2 >= (start + 1)->first - 1) {
    store_wall_range(map_renderer, segment, start->last + 1,
                     (start + 1)->first - 1, a0, a1);
    start++;
    if (x2 <= start->last) {
      return;
    }
  }
  store_wall_range(map_renderer, segment, start->last + 1, x2, a0, a1);
#else
  ClipRange* start = map_renderer->solidsegs;
  ClipRange* next;
  while (start->last < x1 - 1) {
    start++;
  }
  if (x1 < start->first) {
    if (x2 < start->first - 1) {
      store_wall_range(map_renderer, segment, x1, x2);
      next = map_renderer->newend;
      map_renderer->newend++;
      while (next != start) {
        *next = *(next - 1);
        next--;
      }
      next->first = x1;
      next->last = x2;
      return;
    }
    store_wall_range(map_renderer, segment, x1, start->first - 1);
    start->first = x1;
  }

  if (x2 <= start->last) {
    return;
  }

  next = start;
  while (x2 >= (next + 1)->first - 1) {
    store_wall_range(map_renderer, segment, next->last + 1,
                     (next + 1)->first - 1);
    next++;
    if (x2 <= next->last) {
      start->last = next->last;
      goto crunch;
    }
  }
  store_wall_range(map_renderer, segment, next->last + 1, x2);
  start->last = x2;
crunch:
  if (next == start) {
    return;
  }
  while (next++ != map_renderer->newend) {
    *++start = *next;
  }
  map_renderer->newend = start + 1;
#endif
}
#endif

void map_renderer_draw_subsector(MapRenderer* map_renderer, int16_t node_id) {
  DoomMap* map = map_renderer->map;
  SubSector sub_sector = map->subsectors[node_id];
  for (size_t i = 0; i < sub_sector.seg_count; ++i) {
    Segment seg = map->segments[sub_sector.first_seg + i];
    Vec2 v1 = map->vertices[seg.start_vertex];
    Vec2 v2 = map->vertices[seg.end_vertex];
    int x = 0, y = 0;
    float a0 = 0, a1 = 0;
    if (config.top_view) {
      if (config.clip_view) {
        if (map_renderer_add_segment(map_renderer, v1, v2, &x, &y, &a0, &a1)) {
          config.segments += 1;
          map_renderer_draw_segment(map_renderer, seg);
        }
      } else {
        config.segments += 1;
        map_renderer_draw_segment(map_renderer, seg);
      }
    }
  }
}

void map_renderer_draw_segment(MapRenderer* map_renderer, Segment seg) {
  DoomMap* map = map_renderer->map;
  Vec2 v1 = map->vertices[seg.start_vertex];
  Vec2 v2 = map->vertices[seg.end_vertex];
  LineDef ld = map->linedefs[seg.linedef];
  char* name = map->sidedefs[ld.front_sidedef].middle_texture;
  srand(hash(name));
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
    map_renderer_draw_subsector(map_renderer, node_id & (~SSECTOR_IDENTIFIER));
    return;
  }

  if (config.top_view && config.debug_bsp_view) {
    map_renderer_draw_node(map_renderer, node_id);
  }
  Node* node = &map_renderer->map->nodes[node_id];
  if (!player_is_on_side(map_renderer->player, node)) {
    map_renderer_draw_bsp_node(map_renderer, node->left_child);
    map_renderer_draw_bsp_node(map_renderer, node->right_child);
  } else {
    map_renderer_draw_bsp_node(map_renderer, node->right_child);
    map_renderer_draw_bsp_node(map_renderer, node->left_child);
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
