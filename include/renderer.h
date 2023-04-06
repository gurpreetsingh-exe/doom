#ifndef RENDERER_H
#define RENDERER_H

#include "image.h"
#include "vec2.h"

typedef struct {
  Image* image;
} Renderer;

Renderer* renderer_init(Image* image);
void renderer_draw_line(Renderer* renderer, Vec2 v0, Vec2 v1, uint32_t color);
void renderer_clear(Renderer* renderer);
void renderer_destroy(Renderer* renderer);

#endif // !RENDERER_H
