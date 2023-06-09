#ifndef VIEW_RENDERER_H
#define VIEW_RENDERER_H

#include "asset_manager.h"
#include "khash.h"
#include "map.h"
#include "player.h"
#include "renderer.h"

#define MAXSEGS 32
// KHASH_MAP_INIT_STR(col, uint32_t);

typedef struct {
  int first;
  int last;
} ClipRange;

typedef struct {
  AssetManager* am;
  DoomMap* map;
  Renderer* renderer;
  Player* player;
  uint32_t half_width;
  uint32_t half_height;
  uint32_t dist_to_screen;
  float* screen_to_angle;
  float* screen_range;
  bool bsp_traverse;
  float* upper_clip;
  float* lower_clip;
  // kh_col_t* color;

  ClipRange* newend;
  ClipRange* solidsegs;
} ViewRenderer;

ViewRenderer* vr_init(AssetManager* am, DoomMap* map, Renderer* renderer,
                      Player* player);
void vr_init_frame(ViewRenderer* vr);
void vr_draw(ViewRenderer* vr);
void vr_draw_bsp_node(ViewRenderer* vr, int16_t node_id);
void vr_draw_subsector(ViewRenderer* vr, int16_t node_id);
void vr_add_line(ViewRenderer* vr, Segment_t* seg);
void vr_clip_pass_wall(ViewRenderer* vr, Segment_t* segment, int x1, int x2,
                       float rw_angle);
void vr_clip_solid_wall(ViewRenderer* vr, Segment_t* segment, int x1, int x2,
                        float rw_angle);
void vr_destroy(ViewRenderer* vr);

#endif // !VIEW_RENDERER_H
