#ifndef VIEW_RENDERER_H
#define VIEW_RENDERER_H

#include "map.h"
#include "player.h"
#include "renderer.h"

#define MAXSEGS 32

typedef struct {
  int first;
  int last;
} ClipRange;

typedef struct {
  DoomMap* map;
  Renderer* renderer;
  Player* player;
  uint32_t half_width;
  uint32_t half_height;
  uint32_t dist_to_screen;
  float* screen_to_angle;

  ClipRange* newend;
  ClipRange* solidsegs;
} ViewRenderer;

ViewRenderer* vr_init(DoomMap* map, Renderer* renderer, Player* player);
void vr_init_frame(ViewRenderer* vr);
void vr_draw(ViewRenderer* vr);
void vr_draw_bsp_node(ViewRenderer* vr, int16_t node_id);
void vr_draw_subsector(ViewRenderer* vr, int16_t node_id);
void vr_add_line(ViewRenderer* vr, Vec2 v0, Vec2 v1, Segment* seg);
void vr_clip_solid_wall(ViewRenderer* vr, Segment* segment, int x1, int x2,
                        float a0, float a1);
void vr_destroy(ViewRenderer* vr);

#endif // !VIEW_RENDERER_H
