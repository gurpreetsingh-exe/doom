#ifndef RENDERER_H
#define RENDERER_H

#include "asset_manager.h"
#include "image.h"
#include "utils.h"
#include "vec2.h"
#include <string.h>

typedef struct {
  Image* image;
} Renderer;

Renderer* renderer_init(Image* image);
void renderer_draw_line(Renderer* renderer, Vec2 v0, Vec2 v1, uint32_t color);
void renderer_line(Renderer* renderer, int x, int y1, int y2, uint32_t color);
void renderer_draw_image(Renderer* renderer, int x, int y, char* image);
void renderer_draw_flat(Renderer* renderer, Player* player, char* tex_id,
                        int light_level, int x, int y1, int y2, int world_z);
void renderer_draw_wall_col(Renderer* renderer, Texture* tex, int tex_col,
                            int x, int y1, int y2, float tex_alt,
                            float inv_scale, int light_level);
void renderer_draw_rect(Renderer* renderer, Vec2 p0, Vec2 p1, uint32_t color);
void renderer_destroy(Renderer* renderer);

FORCE_INLINE void renderer_draw_point(Renderer* renderer, Vec2 p,
                                      uint32_t color) {
  PIXEL(renderer->image, p.x, p.y) = color;
}

FORCE_INLINE void renderer_clear(Renderer* renderer) {
  memset(renderer->image->data, 0,
         renderer->image->width * renderer->image->height * sizeof(uint32_t));
}

FORCE_INLINE void renderer_resize(Renderer* renderer, uint32_t width,
                                  uint32_t height) {
  image_resize(renderer->image, width, height);
}

FORCE_INLINE void renderer_submit(Renderer* renderer) {
  image_set_data(renderer->image);
}

#endif // !RENDERER_H
