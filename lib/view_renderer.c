#include "view_renderer.h"
#include "config.h"
#include <limits.h>
#include <stdlib.h>

/// right = front
/// left = back

extern Window* window;

int hash(char name[8]) {
  int hash = 593049;
  for (int i = 0; i < 7; ++i) {
    hash += (hash << 5) + name[i];
  }
  return hash;
}

ViewRenderer* vr_init(DoomMap* map, Renderer* renderer, Player* player) {
  ViewRenderer* vr = malloc(sizeof(ViewRenderer));
  vr->map = map;
  vr->renderer = renderer;
  vr->player = player;
  vr->solidsegs = malloc(MAXSEGS * sizeof(ClipRange));
  vr->newend = NULL;
  vr->half_width = window->width / 4;
  vr->half_height = window->height / 4;
  vr->dist_to_screen = vr->half_width / tanf(RADIANS(HALF_FOV));
  vr->screen_to_angle = malloc(sizeof(float) * (window->width / 2 + 1));
  for (size_t i = 0; i <= window->width / 2; ++i) {
    vr->screen_to_angle[i] = norm_angle(
        DEGREES(atan(((float)vr->half_width - i) / (float)vr->dist_to_screen)));
  }
  return vr;
}

void vr_init_frame(ViewRenderer* vr) {
  vr->solidsegs[0].first = INT_MIN;
  vr->solidsegs[0].last = -1;

  vr->solidsegs[1].first = vr->renderer->image->width;
  vr->solidsegs[1].last = INT_MAX;

  vr->newend = vr->solidsegs + 2;
}

void vr_draw(ViewRenderer* vr) {
  DoomMap* map = vr->map;
  vr_draw_bsp_node(vr, map->numnodes - 1);
}

void vr_draw_subsector(ViewRenderer* vr, int16_t node_id) {
  DoomMap* map = vr->map;
  SubSector sub_sector = map->subsectors[node_id];
  for (size_t i = 0; i < sub_sector.seg_count; ++i) {
    Segment seg = map->segments[sub_sector.first_seg + i];
    Vec2 v1 = map->vertices[seg.start_vertex];
    Vec2 v2 = map->vertices[seg.end_vertex];
    vr_add_line(vr, v1, v2, &seg);
  }
}

void vr_add_line(ViewRenderer* vr, Vec2 v0, Vec2 v1, Segment* seg) {
  Player* player = vr->player;
  float a0 = player_angle_to_vec(player, v0);
  float a1 = player_angle_to_vec(player, v1);
  float span = norm_angle(a0 - a1);
  if (span >= 180) {
    return;
  }

  a0 = a0 - player->angle;
  a1 = a1 - player->angle;

  float span1 = norm_angle(a0 + HALF_FOV);
  if (span1 > FOV) {
    if (span1 >= span + FOV) {
      return;
    }
    a0 = HALF_FOV;
  }

  float span2 = norm_angle(HALF_FOV - a1);
  if (span2 > FOV) {
    if (span2 >= span + FOV) {
      return;
    }
    a1 = -HALF_FOV;
  }

  int x1 = angle_to_screen(a0);
  int x2 = angle_to_screen(a1);

  if (x1 == x2) {
    return;
  }

  DoomMap* map = vr->map;
  LineDef ld = map->linedefs[seg->linedef];
  /// right = front
  /// left = back
  if (seg->direction) {
    if (ld.front_sidedef == -1) {
      goto clipsolid;
    }
  } else {
    if (ld.back_sidedef == -1) {
      goto clipsolid;
    }
  }

  // if (seg->direction) {
  //   if (ld.front_sidedef == -1) {
  //     vr_clip_solid_wall(vr, seg, x1, x2, a0, a1);
  //   }
  // } else {
  //   if (ld.back_sidedef == -1) {
  //     vr_clip_solid_wall(vr, seg, x1, x2, a0, a1);
  //   }
  // }

  Sector* backsector = &map->sectors[map->sidedefs[ld.back_sidedef].sector_num];
  Sector* frontsector =
      &map->sectors[map->sidedefs[ld.front_sidedef].sector_num];
  if (backsector->ceiling_height <= frontsector->floor_height ||
      backsector->floor_height >= frontsector->ceiling_height) {
    goto clipsolid;
  }

  if (backsector->ceiling_height != frontsector->ceiling_height ||
      backsector->floor_height != frontsector->floor_height) {
    goto clippass;
  }

clippass:
  vr_clip_pass_wall(vr, seg, x1, x2 - 1, a0, a1);
  return;
clipsolid:
  vr_clip_solid_wall(vr, seg, x1, x2 - 1, a0, a1);
  return;
}

static void calc_height(ViewRenderer* vr, Segment* segment, int x1,
                        float dist_to_v, int* ceiling_v_on_screen,
                        int* floor_v_on_screen) {
  DoomMap* map = vr->map;
  LineDef ld = map->linedefs[segment->linedef];
  Sector sec;
  /// right = front
  /// left = back
  SideDef sd = map->sidedefs[ld.front_sidedef];
  sec = map->sectors[sd.sector_num];

  int ceiling = sec.ceiling_height - vr->player->z;
  int floor = sec.floor_height - vr->player->z;
  float screen_angle = vr->screen_to_angle[x1];
  float dist_to_screen = vr->dist_to_screen / cosf(RADIANS(screen_angle));

  *ceiling_v_on_screen = (abs(ceiling) * dist_to_screen) / dist_to_v;
  *floor_v_on_screen = (abs(floor) * dist_to_screen) / dist_to_v;

  if (ceiling > 0) {
    *ceiling_v_on_screen = vr->half_height - *ceiling_v_on_screen;
  } else {
    *ceiling_v_on_screen += vr->half_height;
  }

  if (floor > 0) {
    *floor_v_on_screen = vr->half_height - *floor_v_on_screen;
  } else {
    *floor_v_on_screen += vr->half_height;
  }
}

static void partial_seg(ViewRenderer* vr, Segment* segment, float* a0,
                        float* a1, float* dist_to_v, bool is_back) {
  DoomMap* map = vr->map;
  Vec2 p0 = map->vertices[segment->start_vertex];
  Vec2 p1 = map->vertices[segment->end_vertex];
  float sidec = sqrt(pow(p0.x - p1.x, 2) + pow(p0.y - p1.y, 2));
  float span = *a0 - *a1;
  float sine_angle = norm_angle(*dist_to_v * sinf(RADIANS(span)) / sidec);
  float angle_b = norm_angle(DEGREES(asinf(sine_angle)));
  float angle_a = norm_angle(norm_angle(180 - span) - angle_b);
  float angle_v_to_fov;
  if (is_back) {
    angle_v_to_fov = norm_angle(*a0 - (vr->player->angle + 45));
  } else {
    angle_v_to_fov = norm_angle((vr->player->angle - 45) - *a1);
  }
  float new_angle = norm_angle(180 - angle_v_to_fov - angle_a);
  *dist_to_v = *dist_to_v * sinf(RADIANS(angle_a)) / sinf(RADIANS(new_angle));
}

static void calc_height_simple(ViewRenderer* vr, Segment* segment, int x1,
                               int x2, float a0, float a1) {
  DoomMap* map = vr->map;
  Vec2 p0 = map->vertices[segment->start_vertex];
  Vec2 p1 = map->vertices[segment->end_vertex];
  float d_to_p0 = player_distance_from_point(vr->player, p0);
  float d_to_p1 = player_distance_from_point(vr->player, p1);
  if (x1 < 0) {
    partial_seg(vr, segment, &a0, &a1, &d_to_p0, true);
  }
  if (x2 >= 320) {
    partial_seg(vr, segment, &a0, &a1, &d_to_p1, false);
  }

  int ceiling_v1_on_screen = 0;
  int floor_v1_on_screen = 0;
  int ceiling_v2_on_screen = 0;
  int floor_v2_on_screen = 0;
  calc_height(vr, segment, x1, d_to_p0, &ceiling_v1_on_screen,
              &floor_v1_on_screen);
  calc_height(vr, segment, x2, d_to_p1, &ceiling_v2_on_screen,
              &floor_v2_on_screen);

  Renderer* renderer = vr->renderer;
  LineDef ld = map->linedefs[segment->linedef];
  char* name = map->sidedefs[ld.front_sidedef].middle_texture;
  srand(hash(name));
  uint8_t r = rand() % 255;
  uint8_t g = rand() % 255;
  uint8_t b = rand() % 255;
  uint8_t a = 255;
  uint32_t color = (a << 24) | (b << 16) | (g << 8) | r;
  if (ceiling_v1_on_screen < 0 || ceiling_v1_on_screen > 199 ||
      ceiling_v2_on_screen < 0 || ceiling_v2_on_screen > 199) {
    return;
  }
  if (floor_v1_on_screen < 0 || floor_v1_on_screen > 199 ||
      floor_v2_on_screen < 0 || floor_v2_on_screen > 199) {
    return;
  }

  renderer_draw_line(renderer, vec2(x1, ceiling_v1_on_screen),
                     vec2(x1, floor_v1_on_screen), color);
  renderer_draw_line(renderer, vec2(x2, ceiling_v2_on_screen),
                     vec2(x2, floor_v2_on_screen), color);
  renderer_draw_line(renderer, vec2(x1, ceiling_v1_on_screen),
                     vec2(x2, ceiling_v2_on_screen), color);
  renderer_draw_line(renderer, vec2(x1, floor_v1_on_screen),
                     vec2(x2, floor_v2_on_screen), color);
}

static void store_wall_range(ViewRenderer* vr, Segment* segment, int x1, int x2,
                             float a0, float a1) {
  calc_height_simple(vr, segment, x1, x2, a0, a1);
}

void vr_clip_pass_wall(ViewRenderer* vr, Segment* segment, int x1, int x2,
                       float a0, float a1) {
  ClipRange* start;
  start = vr->solidsegs;
  while (start->last < x1 - 1) {
    start++;
  }
  if (x1 < start->first) {
    if (x2 < start->first - 1) {
      store_wall_range(vr, segment, x1, x2, a0, a1);
      return;
    }
    store_wall_range(vr, segment, x1, start->first - 1, a0, a1);
  }
  if (x2 <= start->last) {
    return;
  }
  while (x2 >= (start + 1)->first - 1) {
    store_wall_range(vr, segment, start->last + 1, (start + 1)->first - 1, a0,
                     a1);
    start++;
    if (x2 <= start->last) {
      return;
    }
  }
  store_wall_range(vr, segment, start->last + 1, x2, a0, a1);
}

void vr_clip_solid_wall(ViewRenderer* vr, Segment* segment, int x1, int x2,
                        float a0, float a1) {
  ClipRange* start = vr->solidsegs;
  ClipRange* next;
  while (start->last < x1 - 1) {
    start++;
  }
  if (x1 < start->first) {
    if (x2 < start->first - 1) {
      store_wall_range(vr, segment, x1, x2, a0, a1);
      next = vr->newend;
      vr->newend++;
      while (next != start) {
        *next = *(next - 1);
        next--;
      }
      next->first = x1;
      next->last = x2;
      return;
    }
    store_wall_range(vr, segment, x1, start->first - 1, a0, a1);
    start->first = x1;
  }

  if (x2 <= start->last) {
    return;
  }

  next = start;
  while (x2 >= (next + 1)->first - 1) {
    store_wall_range(vr, segment, next->last + 1, (next + 1)->first - 1, a0,
                     a1);
    next++;
    if (x2 <= next->last) {
      start->last = next->last;
      goto crunch;
    }
  }
  store_wall_range(vr, segment, next->last + 1, x2, a0, a1);
  start->last = x2;
crunch:
  if (next == start) {
    return;
  }
  while (next++ != vr->newend) {
    *++start = *next;
  }
  vr->newend = start + 1;
}

void vr_draw_bsp_node(ViewRenderer* vr, int16_t node_id) {
  if (node_id & SSECTOR_IDENTIFIER) {
    vr_draw_subsector(vr, node_id & (~SSECTOR_IDENTIFIER));
    return;
  }

  Node* node = &vr->map->nodes[node_id];
  if (player_is_on_side(vr->player, node)) {
    vr_draw_bsp_node(vr, node->left_child);
    vr_draw_bsp_node(vr, node->right_child);
  } else {
    vr_draw_bsp_node(vr, node->right_child);
    vr_draw_bsp_node(vr, node->left_child);
  }
}

void vr_destroy(ViewRenderer* vr) {
  free(vr->screen_to_angle);
  free(vr->solidsegs);
  free(vr);
}
