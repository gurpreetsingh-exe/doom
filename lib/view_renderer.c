#include "view_renderer.h"
#include "config.h"
#include <limits.h>
#include <stdlib.h>

#define DBG 1

KHASH_MAP_INIT_STR(col, uint32_t);

extern Window* window;
extern Config config;

static kh_col_t* color;

int hash(char* name) {
  int hash = 0;
  for (int i = 0; i < 8; ++i) {
    hash += (hash << 5) + name[i];
  }
  return hash;
}

static bool eq(char* a, char* b, int sz) { return strncmp(a, b, sz) == 0; }

ViewRenderer* vr_init(DoomMap* map, Renderer* renderer, Player* player) {
  ViewRenderer* vr = malloc(sizeof(ViewRenderer));
  vr->map = map;
  vr->renderer = renderer;
  vr->player = player;
  vr->solidsegs = malloc(MAXSEGS * sizeof(ClipRange));
  vr->newend = NULL;
  vr->half_width = window->width / 2;
  vr->half_height = window->height / 2;
  vr->dist_to_screen = (float)vr->half_width;
  vr->screen_to_angle = malloc(sizeof(float) * (window->width + 1));
  vr->screen_range = malloc(sizeof(float) * window->width);

  vr->upper_clip = malloc(sizeof(float) * window->width);
  vr->lower_clip = malloc(sizeof(float) * window->width);
  color = kh_init(col);

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
    vr->screen_range[i] = i;
    vr->upper_clip[i] = -1;
    vr->lower_clip[i] = window->height;
  }
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
  Sector* backsector = seg->back_sector;
  Sector* frontsector = seg->front_sector;

  if (backsector == NULL) {
    // goto clipsolid;
    vr_clip_solid_wall(vr, seg, x1, x2 - 1, rw_angle);
    return;
  }

  if (backsector->ceiling_height <= frontsector->floor_height ||
      backsector->floor_height >= frontsector->ceiling_height) {
    // goto clipsolid;
    vr_clip_solid_wall(vr, seg, x1, x2 - 1, rw_angle);
    return;
  }

  if (backsector->ceiling_height != frontsector->ceiling_height ||
      backsector->floor_height != frontsector->floor_height) {
    // goto clippass;
    vr_clip_pass_wall(vr, seg, x1, x2 - 1, rw_angle);
    return;
  }

  // if (eq(backsector->ceiling_texture, frontsector->ceiling_texture, 8) &&
  //     eq(backsector->floor_texture, frontsector->floor_texture, 8) &&
  //     backsector->light_level == frontsector->light_level &&
  //     seg->linedef->front_sidedef->middle_texture[0] == '-') {
  //   return;
  // }

  vr_clip_pass_wall(vr, seg, x1, x2 - 1, rw_angle);
  return;

clippass:
  config.segments += 1;
  vr_clip_pass_wall(vr, seg, x1, x2 - 1, rw_angle);
  return;
clipsolid:
  config.segments += 1;
  vr_clip_solid_wall(vr, seg, x1, x2 - 1, rw_angle);
  return;
}

static void calc_height(ViewRenderer* vr, Segment_t* segment, int x1,
                        float dist_to_v, int* ceiling_v_on_screen,
                        int* floor_v_on_screen) {
  Sector* sec = segment->linedef->front_sidedef->sector;

  int ceiling = sec->ceiling_height - vr->player->z;
  int floor = sec->floor_height - vr->player->z;
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

static void partial_seg(ViewRenderer* vr, Segment_t* segment, float* a0,
                        float* a1, float* dist_to_v, bool is_back) {
  Vec2 p0 = *(segment->start_vertex);
  Vec2 p1 = *(segment->end_vertex);
  float sidec = sqrt(pow(p0.x - p1.x, 2) + pow(p0.y - p1.y, 2));
  float span = norm_angle(*a0 - *a1);
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

static void calc_height_simple(ViewRenderer* vr, Segment_t* segment, int x1,
                               int x2, float a0, float a1) {
  // return;
  Vec2 p0 = *(segment->start_vertex);
  Vec2 p1 = *(segment->end_vertex);
  float d_to_p0 = player_distance_from_point(vr->player, p0);
  float d_to_p1 = player_distance_from_point(vr->player, p1);
  if (x1 <= 0) {
    partial_seg(vr, segment, &a0, &a1, &d_to_p0, true);
  }
  if (x2 >= 319) {
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
  LineDef_t* ld = segment->linedef;
  char* name = ld->front_sidedef->middle_texture;
  uint8_t r = rand() % 255;
  uint8_t g = rand() % 255;
  uint8_t b = rand() % 255;
  uint8_t a = 255;
  uint32_t color = (a << 24) | (b << 16) | (g << 8) | r;

  renderer_draw_line(renderer, vec2(x1, ceiling_v1_on_screen),
                     vec2(x1, floor_v1_on_screen), color);
  renderer_draw_line(renderer, vec2(x2, ceiling_v2_on_screen),
                     vec2(x2, floor_v2_on_screen), color);
  renderer_draw_line(renderer, vec2(x1, ceiling_v1_on_screen),
                     vec2(x2, ceiling_v2_on_screen), color);
  renderer_draw_line(renderer, vec2(x1, floor_v1_on_screen),
                     vec2(x2, floor_v2_on_screen), color);
}

static float scale_factor(ViewRenderer* vr, int x, float angle, float dist) {
  float a = vr->screen_to_angle[x];
  float num = vr->dist_to_screen * cosf(RADIANS(angle - a - vr->player->angle));
  float den = dist * cosf(RADIANS(a));
  float scale = num / den;
  scale = MIN(64.0, MAX(0.00390625, scale));
  return scale;
}

static uint32_t get_color(char* id, int light_level) {
#if 0
  char key[32] = {0};
  sprintf(key, "%s%d", id, light_level);
  khint_t k = kh_get(col, color, key);
  if (k == kh_end(color)) {
    srand(hash(id));
    int absent;
    khint_t k = kh_put(col, color, key, &absent);
    // light_level /= 255;
    uint8_t r = (rand() % 255) * light_level;
    uint8_t g = (rand() % 255) * light_level;
    uint8_t b = (rand() % 255) * light_level;
    uint8_t a = 255;
    uint32_t _c = (a << 24) | (b << 16) | (g << 8) | r;
    kh_value(color, k) = _c;
    return _c;
  }
  return kh_value(color, k);
#else
  srand(hash(id));
  uint8_t r = (rand() % 255) * light_level;
  uint8_t g = (rand() % 255) * light_level;
  uint8_t b = (rand() % 255) * light_level;
  uint8_t a = 255;
  return (a << 24) | (b << 16) | (g << 8) | r;
#endif
}

static void store_wall_range(ViewRenderer* vr, Segment_t* segment, int x1,
                             int x2, float rw_angle) {

  Sector* front = segment->front_sector;
  Sector* back = segment->back_sector;
  LineDef_t* ld = segment->linedef;
  SideDef_t* sd = ld->front_sidedef;

  float rw_normal_angle = segment->angle + 90;
  float off_angle = fabsf(rw_normal_angle - rw_angle);
  if (off_angle > 90) {
    off_angle = 90;
  }

  float distance = 90 - off_angle;
  float hyp = player_distance_from_point(vr->player, *segment->start_vertex);
  float rw_dist = distance * sinf(RADIANS(distance));
  float rw_scale_step;

  float rw_scale1 = scale_factor(vr, x1, rw_normal_angle, rw_dist);
  if (x2 > x1) {
    float rw_scale2 = scale_factor(vr, x2, rw_normal_angle, rw_dist);
    rw_scale_step = (rw_scale2 - rw_scale1) / (x2 - x1);
  } else {
    rw_scale_step = 0;
  }

  float worldtop = front->ceiling_height - vr->player->z;
  float worldbottom = front->floor_height - vr->player->z;
  char* midtexture = 0;
  char* toptexture = 0;
  char* bottomtexture = 0;
  char* maskedtexture = 0;

  bool markfloor, markceiling;
  if (!back) {
    midtexture = sd->middle_texture;
    markfloor = markceiling = true;
  }
}

static void store_portal_wall_range(ViewRenderer* vr, Segment_t* segment,
                                    int x1, int x2, float rw_angle) {
#if DBG
  Sector* front = segment->front_sector;
  Sector* back = segment->back_sector;
  LineDef_t* ld = segment->linedef;
  SideDef_t* sd = ld->front_sidedef;

  char* upper_wall_texture = sd->upper_texture;
  char* lower_wall_texture = sd->lower_texture;
  char* tex_ceil_id = front->ceiling_texture;
  char* tex_floor_id = front->floor_texture;

  int world_front_z1 = front->ceiling_height - vr->player->z;
  int world_back_z1 = back->ceiling_height - vr->player->z;
  int world_front_z2 = front->floor_height - vr->player->z;
  int world_back_z2 = back->floor_height - vr->player->z;
  int light_level = front->light_level;

  bool b_draw_upper_wall = false;
  bool b_draw_ceil = true;
  if (world_front_z1 != world_back_z1 ||
      front->light_level != back->light_level ||
      !eq(front->ceiling_texture, back->ceiling_texture, 8)) {
    // b_draw_upper_wall = sd->upper_texture[0] != '-';
    // b_draw_upper_wall = world_back_z1 < world_front_z1;
    b_draw_ceil = world_front_z1 >= 0;
  } else {
    b_draw_upper_wall = false;
    b_draw_ceil = false;
  }

  // bool b_draw_upper_wall = false;
  // bool b_draw_ceil = true;
  // if (world_front_z1 != world_back_z1 ||
  //     front->light_level != back->light_level ||
  //     !eq(front->ceiling_texture, back->ceiling_texture, 8)) {
  //   // b_draw_upper_wall =
  //   //     sd->upper_texture[0] != '-' && world_back_z1 < world_front_z1;
  //   b_draw_ceil = world_front_z1 >= 0;
  // } else {
  //   b_draw_ceil = false;
  // }

  bool b_draw_lower_wall = false;
  bool b_draw_floor = false;
  if (world_front_z2 != world_back_z2 ||
      front->light_level != back->light_level ||
      !eq(front->floor_texture, back->floor_texture, 8)) {
    b_draw_lower_wall =
        sd->lower_texture[0] != '-' && world_back_z2 > world_front_z2;
    // b_draw_lower_wall = world_back_z2 > world_front_z2;
    b_draw_floor = world_front_z2 <= 0;
  } else {
    b_draw_lower_wall = false;
    b_draw_floor = false;
  }

  if (!b_draw_upper_wall && !b_draw_lower_wall && !b_draw_ceil &&
      !b_draw_floor) {
    return;
  }

  float angle = segment->angle + 90;
  float off_angle = angle - rw_angle;
  float hp = player_distance_from_point(vr->player, *segment->start_vertex);
  float rw_dist = hp * cosf(RADIANS(off_angle));
  float rw_scale1 = scale_factor(vr, x1, angle, rw_dist);
  float rw_scale_step = 0;

  if (x1 < x2) {
    float scale2 = scale_factor(vr, x2, angle, rw_dist);
    rw_scale_step = (scale2 - rw_scale1) / (x2 - x1);
  }

  float wall_y1 = vr->half_height - world_front_z1 * rw_scale1;
  float wall_y1_step = -rw_scale_step * world_front_z1;

  float wall_y2 = vr->half_height - world_front_z2 * rw_scale1;
  float wall_y2_step = -rw_scale_step * world_front_z2;

  float portal_y1, portal_y1_step;
  // if (b_draw_upper_wall) {
  //   if (world_back_z1 > world_front_z2) {
  //     portal_y1 = vr->half_height - world_back_z1 * rw_scale1;
  //     portal_y1_step = -rw_scale_step * world_back_z1;
  //   } else {
  //     portal_y1 = wall_y2;
  //     portal_y1_step = wall_y2_step;
  //   }
  // }

  float portal_y2, portal_y2_step;
  if (b_draw_lower_wall) {
    if (world_back_z2 < world_front_z1) {
      portal_y2 = vr->half_height - world_back_z2 * rw_scale1;
      portal_y2_step = -rw_scale_step * world_back_z2;
    } else {
      portal_y2 = wall_y1;
      portal_y2_step = wall_y1_step;
    }
  }

  uint32_t c0 = get_color(tex_ceil_id, light_level);
  uint32_t c1 = get_color(upper_wall_texture, light_level);
  uint32_t c2 = get_color(tex_floor_id, light_level);
  uint32_t c3 = get_color(lower_wall_texture, light_level);
  for (int i = x1; i <= x2; ++i) {
    float draw_wall_y1 = wall_y1 - 1;
    float draw_wall_y2 = wall_y2;

    // if (b_draw_upper_wall) {
    //   float draw_upper_wall_y1 = wall_y1 - 1;
    //   float draw_upper_wall_y2 = portal_y1;
    //
    //   if (b_draw_ceil) {
    //     float cy1 = vr->upper_clip[i] + 1;
    //     float cy2 = MIN(draw_wall_y1 - 1, vr->lower_clip[i] - 1);
    //     renderer_line(vr->renderer, i, cy1, cy2, c0);
    //   }
    //
    //   float wy1 = MAX(draw_upper_wall_y1, vr->upper_clip[i] + 1);
    //   float wy2 = MAX(draw_upper_wall_y2, vr->lower_clip[i] - 1);
    //   renderer_line(vr->renderer, i, wy1, wy2, c1);
    //   if (vr->upper_clip[i] < wy2) {
    //     vr->upper_clip[i] = wy2;
    //   }
    //   portal_y1 += portal_y1_step;
    // }

    if (b_draw_ceil) {
      float cy1 = vr->upper_clip[i] + 1;
      float cy2 = MIN(draw_wall_y1 - 1, vr->lower_clip[i] - 1);
      renderer_line(vr->renderer, i, cy1, cy2, c0);
      if (vr->upper_clip[i] < cy2) {
        vr->upper_clip[i] = cy2;
      }
    }

    if (b_draw_lower_wall) {
      if (b_draw_floor) {
        float fy1 = MAX(draw_wall_y2 + 1, vr->upper_clip[i] + 1);
        float fy2 = vr->lower_clip[i] - 1;
        renderer_line(vr->renderer, i, fy1, fy2, c2);
      }
      float draw_lower_wall_y1 = portal_y2 - 1;
      float draw_lower_wall_y2 = wall_y2;
      float wy1 = MAX(draw_lower_wall_y1, vr->upper_clip[i] + 1);
      float wy2 = MIN(draw_lower_wall_y2, vr->lower_clip[i] - 1);
      renderer_line(vr->renderer, i, wy1, wy2, c3);
      if (vr->lower_clip[i] > wy1) {
        vr->lower_clip[i] = wy1;
      }
      portal_y2 += portal_y2_step;
    }

    if (b_draw_floor) {
      float fy1 = MAX(draw_wall_y2 + 1, vr->upper_clip[i] + 1);
      float fy2 = vr->lower_clip[i] - 1;
      renderer_line(vr->renderer, i, fy1, fy2, c2);
      if (vr->lower_clip[i] > draw_wall_y2 + 1) {
        vr->lower_clip[i] = fy1;
      }
    }
    wall_y1 += wall_y1_step;
    wall_y2 += wall_y2_step;
  }
#else
  store_wall_range(vr, segment, x1, x2, rw_angle);
#endif
}

static void store_solid_wall_range(ViewRenderer* vr, Segment_t* segment, int x1,
                                   int x2, float rw_angle) {
#if DBG
  Sector* front = segment->front_sector;

  float angle = segment->angle + 90;
  float off_angle = angle - rw_angle;
  float hp = player_distance_from_point(vr->player, *segment->start_vertex);
  float rw_dist = hp * cosf(RADIANS(off_angle));
  float rw_scale1 = scale_factor(vr, x1, angle, rw_dist);
  float rw_scale_step = 0;
  float z1 = front->ceiling_height - vr->player->z;
  float z2 = front->floor_height - vr->player->z;

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

  char* wall_texture = segment->linedef->front_sidedef->middle_texture;
  char* ceil_texture = front->ceiling_texture;
  char* floor_texture = front->floor_texture;
  int light_level = front->light_level;

  bool b_draw_wall = wall_texture[0] != '-';
  bool b_draw_ceil = z1 > 0;
  bool b_draw_floor = z2 < 0;
  uint32_t c0 = get_color(ceil_texture, light_level);
  uint32_t c1 = get_color(wall_texture, light_level);
  uint32_t c2 = get_color(floor_texture, light_level);

  for (int i = x1; i <= x2; ++i) {
    float draw_wall_y1 = wall_y1 - 1;
    float draw_wall_y2 = wall_y2;
    if (b_draw_ceil) {
      float cy1 = vr->upper_clip[i] + 1;
      float cy2 = MIN(draw_wall_y1 - 1, vr->lower_clip[i] - 1);
      renderer_line(vr->renderer, i, cy1, cy2, c0);
    }

    if (b_draw_wall) {
      float wy1 = MAX(draw_wall_y1, vr->upper_clip[i] + 1);
      float wy2 = MIN(draw_wall_y2, vr->lower_clip[i] - 1);
      renderer_line(vr->renderer, i, wy1, wy2, c1);
    }

    if (b_draw_floor) {
      float fy1 = MAX(draw_wall_y2 + 1, vr->upper_clip[i] + 1);
      float fy2 = vr->lower_clip[i] - 1;
      renderer_line(vr->renderer, i, fy1, fy2, c2);
    }
    wall_y1 += wall_y1_step;
    wall_y2 += wall_y2_step;
  }
#else
  store_wall_range(vr, segment, x1, x2, rw_angle);
#endif
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
      store_portal_wall_range(vr, segment, x1, x2, rw_angle);
      return;
    }
    store_portal_wall_range(vr, segment, x1, start->first - 1, rw_angle);
  }
  if (x2 <= start->last) {
    return;
  }
  while (x2 >= (start + 1)->first - 1) {
    store_portal_wall_range(vr, segment, start->last + 1,
                            (start + 1)->first - 1, rw_angle);
    start++;
    if (x2 <= start->last) {
      return;
    }
  }
  store_portal_wall_range(vr, segment, start->last + 1, x2, rw_angle);
}

void vr_clip_solid_wall(ViewRenderer* vr, Segment_t* segment, int x1, int x2,
                        float rw_angle) {
  ClipRange* start = vr->solidsegs;
  ClipRange* next;
  while (start->last < x1 - 1) {
    start++;
  }
  if (x1 < start->first) {
    if (x2 < start->first - 1) {
      store_solid_wall_range(vr, segment, x1, x2, rw_angle);
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
    store_solid_wall_range(vr, segment, x1, start->first - 1, rw_angle);
    start->first = x1;
  }

  if (x2 <= start->last) {
    return;
  }

  next = start;
  while (x2 >= (next + 1)->first - 1) {
    store_solid_wall_range(vr, segment, next->last + 1, (next + 1)->first - 1,
                           rw_angle);
    next++;
    if (x2 <= next->last) {
      start->last = next->last;
      goto crunch;
    }
  }
  store_solid_wall_range(vr, segment, next->last + 1, x2, rw_angle);
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
