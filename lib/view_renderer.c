#include "view_renderer.h"
#include "config.h"
#include <limits.h>
#include <stdlib.h>

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
  vr->half_width = window->width / 2;
  vr->half_height = window->height / 2;
  vr->dist_to_screen = (float)vr->half_width / tanf(RADIANS(HALF_FOV));
  vr->screen_to_angle = malloc(sizeof(float) * (window->width + 1));
  khash_t(f32)* screen_range = kh_init(f32);
  vr->screen_range = screen_range;

  for (size_t i = 0; i <= window->width; ++i) {
    vr->screen_to_angle[i] =
        DEGREES(atan(((float)vr->half_width - i) / (float)vr->dist_to_screen));
  }
  return vr;
}

void vr_init_frame(ViewRenderer* vr) {
  vr->solidsegs[0].first = INT_MIN;
  vr->solidsegs[0].last = -1;

  vr->solidsegs[1].first = vr->renderer->image->width;
  vr->solidsegs[1].last = INT_MAX;

  vr->newend = vr->solidsegs + 2;

  for (size_t i = 0; i < window->width; ++i) {
    int absent;
    kh_put(f32, vr->screen_range, i, &absent);
  }
  // kh_del(f32, vr->screen_range, 50);
  // int val = 50;
  // int a = kh_get(f32, vr->screen_range, val);
  // printf("%d\n", a == val);
}

void vr_draw(ViewRenderer* vr) {
  DoomMap* map = vr->map;
  vr_draw_bsp_node(vr, map->numnodes - 1);
}

void vr_draw_subsector(ViewRenderer* vr, int16_t node_id) {
  DoomMap* map = vr->map;
  SubSector sub_sector = map->subsectors[node_id];
  for (size_t i = 0; i < sub_sector.seg_count; ++i) {
    Segment_t seg = map->segments_t[sub_sector.first_seg + i];
    vr_add_line(vr, &seg);
  }
}

void vr_add_line(ViewRenderer* vr, Segment_t* seg) {
  Vec2* v0 = seg->start_vertex;
  Vec2* v1 = seg->end_vertex;
  Player* player = vr->player;
  float a0 = player_angle_to_vec(player, *v0);
  float a1 = player_angle_to_vec(player, *v1);
  float span = norm_angle(a0 - a1);
  if (span >= 180) {
    return;
  }
  float rw_angle = a0;

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

  LineDef_t* ld = seg->linedef;
  if (ld->back_sidedef == NULL) {
    goto clipsolid;
  }

  Sector* backsector = seg->back_sector;
  Sector* frontsector = seg->front_sector;
  if (backsector->ceiling_height != frontsector->ceiling_height ||
      backsector->floor_height != frontsector->floor_height) {
    goto clippass;
  }

  // if (memcmp(backsector->floor_texture, frontsector->floor_texture, 8) == 0
  // &&
  //     memcmp(backsector->ceiling_texture, frontsector->ceiling_texture, 8) ==
  //         0 &&
  //     backsector->light_level == frontsector->light_level &&
  //     seg->linedef->front_sidedef->middle_texture[0] == '-') {
  //   return;
  // }

clippass:
  vr_clip_pass_wall(vr, seg, x1, x2 - 1, rw_angle);
  return;
clipsolid:
  vr_clip_solid_wall(vr, seg, x1, x2 - 1, rw_angle);
  return;
}

// static void calc_height(ViewRenderer* vr, Segment_t* segment, int x1,
//                         float dist_to_v, int* ceiling_v_on_screen,
//                         int* floor_v_on_screen) {
//   Sector* sec = segment->linedef->front_sidedef->sector;
//
//   int ceiling = sec->ceiling_height - vr->player->z;
//   int floor = sec->floor_height - vr->player->z;
//   float screen_angle = vr->screen_to_angle[x1];
//   float dist_to_screen = vr->dist_to_screen / cosf(RADIANS(screen_angle));
//
//   *ceiling_v_on_screen = (abs(ceiling) * dist_to_screen) / dist_to_v;
//   *floor_v_on_screen = (abs(floor) * dist_to_screen) / dist_to_v;
//
//   if (ceiling > 0) {
//     *ceiling_v_on_screen = vr->half_height - *ceiling_v_on_screen;
//   } else {
//     *ceiling_v_on_screen += vr->half_height;
//   }
//
//   if (floor > 0) {
//     *floor_v_on_screen = vr->half_height - *floor_v_on_screen;
//   } else {
//     *floor_v_on_screen += vr->half_height;
//   }
// }
//
// static void partial_seg(ViewRenderer* vr, Segment_t* segment, float* a0,
//                         float* a1, float* dist_to_v, bool is_back) {
//   Vec2 p0 = *(segment->start_vertex);
//   Vec2 p1 = *(segment->end_vertex);
//   float sidec = sqrt(pow(p0.x - p1.x, 2) + pow(p0.y - p1.y, 2));
//   float span = *a0 - *a1;
//   float sine_angle = norm_angle(*dist_to_v * sinf(RADIANS(span)) / sidec);
//   float angle_b = norm_angle(DEGREES(asinf(sine_angle)));
//   float angle_a = norm_angle(norm_angle(180 - span) - angle_b);
//   float angle_v_to_fov;
//   if (is_back) {
//     angle_v_to_fov = norm_angle(*a0 - (vr->player->angle + 45));
//   } else {
//     angle_v_to_fov = norm_angle((vr->player->angle - 45) - *a1);
//   }
//   float new_angle = norm_angle(180 - angle_v_to_fov - angle_a);
//   *dist_to_v = *dist_to_v * sinf(RADIANS(angle_a)) /
//   sinf(RADIANS(new_angle));
// }
//
// static void calc_height_simple(ViewRenderer* vr, Segment_t* segment, int x1,
//                                int x2, float a0, float a1) {
//   return;
//   Vec2 p0 = *(segment->start_vertex);
//   Vec2 p1 = *(segment->end_vertex);
//   float d_to_p0 = player_distance_from_point(vr->player, p0);
//   float d_to_p1 = player_distance_from_point(vr->player, p1);
//   if (x1 < 0) {
//     partial_seg(vr, segment, &a0, &a1, &d_to_p0, true);
//   }
//   if (x2 >= 319) {
//     partial_seg(vr, segment, &a0, &a1, &d_to_p1, false);
//   }
//
//   int ceiling_v1_on_screen = 0;
//   int floor_v1_on_screen = 0;
//   int ceiling_v2_on_screen = 0;
//   int floor_v2_on_screen = 0;
//   calc_height(vr, segment, x1, d_to_p0, &ceiling_v1_on_screen,
//               &floor_v1_on_screen);
//   calc_height(vr, segment, x2, d_to_p1, &ceiling_v2_on_screen,
//               &floor_v2_on_screen);
//
//   Renderer* renderer = vr->renderer;
//   LineDef_t* ld = segment->linedef;
//   char* name = ld->front_sidedef->middle_texture;
//   srand(hash(name));
//   uint8_t r = rand() % 255;
//   uint8_t g = rand() % 255;
//   uint8_t b = rand() % 255;
//   uint8_t a = 255;
//   uint32_t color = (a << 24) | (b << 16) | (g << 8) | r;
//   if (ceiling_v1_on_screen < 0 || ceiling_v1_on_screen > 199 ||
//       ceiling_v2_on_screen < 0 || ceiling_v2_on_screen > 199) {
//     return;
//   }
//   if (floor_v1_on_screen < 0 || floor_v1_on_screen > 199 ||
//       floor_v2_on_screen < 0 || floor_v2_on_screen > 199) {
//     return;
//   }
//
//   renderer_draw_line(renderer, vec2(x1, ceiling_v1_on_screen),
//                      vec2(x1, floor_v1_on_screen), color);
//   renderer_draw_line(renderer, vec2(x2, ceiling_v2_on_screen),
//                      vec2(x2, floor_v2_on_screen), color);
//   renderer_draw_line(renderer, vec2(x1, ceiling_v1_on_screen),
//                      vec2(x2, ceiling_v2_on_screen), color);
//   renderer_draw_line(renderer, vec2(x1, floor_v1_on_screen),
//                      vec2(x2, floor_v2_on_screen), color);
// }

static float scale_factor(ViewRenderer* vr, int x, float angle, float dist) {
  float a = vr->screen_to_angle[x];
  float num = vr->dist_to_screen * cosf(RADIANS(angle - a - vr->player->angle));
  float den = dist * cosf(RADIANS(a));
  float scale = num / den;
  scale = MIN(64.0, MAX(0.00390625, scale));
  return scale;
}

static void store_wall_range(ViewRenderer* vr, Segment_t* segment, int x1,
                             int x2, float rw_angle) {
  float angle = segment->angle + 90;
  float off_angle = angle - rw_angle;
  float hp = player_distance_from_point(vr->player, *segment->start_vertex);
  float rw_dist = hp * cosf(RADIANS(off_angle));
  float rw_scale1 = scale_factor(vr, x1, angle, rw_dist);
  float rw_scale_step = 0;
  // float z1 = segment->front_sector->ceiling_height - vr->player->z;
  // float z2 = segment->front_sector->floor_height - vr->player->z;
  float z1 = segment->front_sector->ceiling_height - 41;
  float z2 = segment->front_sector->floor_height - 41;

  if (x1 < x2) {
    float scale2 = scale_factor(vr, x2, angle, rw_dist);
    rw_scale_step = (scale2 - rw_scale1) / (x2 - x1);
  } else {
    rw_scale_step = 0;
  }

  float wall_y1 = vr->half_height - z1 * rw_scale1;
  float wall_y1_step = -rw_scale_step * z1;

  float wall_y2 = vr->half_height - z2 * rw_scale1;
  float wall_y2_step = -rw_scale_step * z2;

  char* name = segment->linedef->front_sidedef->middle_texture;
  srand(hash(name));
  uint8_t r = rand() % 255;
  uint8_t g = rand() % 255;
  uint8_t b = rand() % 255;
  uint8_t a = 255;
  uint32_t color = (a << 24) | (b << 16) | (g << 8) | r;

  for (int i = x1; i < x2; ++i) {
    float draw_wall_y1 = wall_y1 - 1;
    float draw_wall_y2 = wall_y2;
    if (segment->linedef->front_sidedef->middle_texture[0] != '-') {
      renderer_draw_line(vr->renderer, vec2(i, draw_wall_y1),
                         vec2(i, draw_wall_y2), color);
    }
    wall_y1 += wall_y1_step;
    wall_y2 += wall_y2_step;
  }
}

void vr_clip_pass_wall(ViewRenderer* vr, Segment_t* segment, int x1, int x2,
                       float rw_angle) {
  ClipRange* start;
  start = vr->solidsegs;
  while (start->last < x1 - 1) {
    start++;
  }
  if (x1 < start->first) {
    if (x2 < start->first - 1) {
      store_wall_range(vr, segment, x1, x2, rw_angle);
      return;
    }
    store_wall_range(vr, segment, x1, start->first - 1, rw_angle);
  }
  if (x2 <= start->last) {
    return;
  }
  while (x2 >= (start + 1)->first - 1) {
    store_wall_range(vr, segment, start->last + 1, (start + 1)->first - 1,
                     rw_angle);
    start++;
    if (x2 <= start->last) {
      return;
    }
  }
  store_wall_range(vr, segment, start->last + 1, x2, rw_angle);
}

void vr_clip_solid_wall(ViewRenderer* vr, Segment_t* segment, int x1, int x2,
                        float rw_angle) {
#if 0
  ClipRange* start = vr->solidsegs;
  ClipRange* next;
  while (start->last < x1 - 1) {
    start++;
  }
  if (x1 < start->first) {
    if (x2 < start->first - 1) {
      store_wall_range(vr, segment, x1, x2, rw_angle);
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
    store_wall_range(vr, segment, x1, start->first - 1, rw_angle);
    start->first = x1;
  }

  if (x2 <= start->last) {
    return;
  }

  next = start;
  while (x2 >= (next + 1)->first - 1) {
    store_wall_range(vr, segment, next->last + 1, (next + 1)->first - 1,
                     rw_angle);
    next++;
    if (x2 <= next->last) {
      start->last = next->last;
      goto crunch;
    }
  }
  store_wall_range(vr, segment, next->last + 1, x2, rw_angle);
  start->last = x2;
crunch:
  if (next == start) {
    return;
  }
  while (next++ != vr->newend) {
    *++start = *next;
  }
  vr->newend = start + 1;
#else
  int i = 0;
  int _;
  (void)_;
  kh_foreach_value(vr->screen_range, _, { i++; });
  if (i > 0) {
    int absent;
    khash_t(f32)* curr_wall = kh_init(f32);
    khash_t(f32)* intersection = kh_init(f32);
    khint_t __i, __j;
    __i = kh_begin(vr->screen_range), __j = kh_begin(curr_wall);
    for (; __i != kh_end(vr->screen_range) || __j != kh_end(curr_wall);
         ++__i, ++__j) {
      if (!kh_exist(vr->screen_range, __i))
        continue;
      if (!kh_exist(curr_wall, __j))
        continue;

      khint_t val = __i;
      if (val != kh_get(f32, vr->screen_range, val)) {
        continue;
      }

      if (val != kh_get(f32, curr_wall, val)) {
        continue;
      }
      kh_put(f32, intersection, val, &absent);
    }
    int __int = 0;
    kh_foreach_value(intersection, _, { __int++; });
    int __curr_w = 0;
    kh_foreach_value(curr_wall, _, { __curr_w++; });
    if (__int == __curr_w) {
      store_wall_range(vr, segment, x1, x2, rw_angle);
    }
  } else {
    vr->bsp_traverse = false;
  }
#endif
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
