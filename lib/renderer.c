#include "renderer.h"
#include "image.h"
#include "wad.h"
#include <stdlib.h>

Renderer* renderer_init(Image* image) {
  Renderer* renderer = (Renderer*)malloc(sizeof(Renderer));
  renderer->image = image;
  return renderer;
}

/// https://rosettacode.org/wiki/Bitmap/Bresenham's_line_algorithm#C
void renderer_draw_line(Renderer* renderer, Vec2 v0, Vec2 v1, uint32_t color) {
  int x0 = v0.x;
  int x1 = v1.x;
  int y0 = v0.y;
  int y1 = v1.y;
  int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = (dx > dy ? dx : -dy) / 2, e2;

  for (;;) {
    renderer->image->data[x0 + (renderer->image->height - y0 - 1) * renderer->image->width] = color;
    if (x0 == x1 && y0 == y1) {
      break;
    }
    e2 = err;
    if (e2 > -dx) {
      err -= dy;
      x0 += sx;
    }
    if (e2 < dy) {
      err += dx;
      y0 += sy;
    }
  }
}

void renderer_draw_rect(Renderer* renderer, Vec2 p0, Vec2 p1, uint32_t color) {
  Vec2 v0 = vec2(p0.x, p0.y);
  Vec2 v1 = vec2(p0.x, p1.y);
  Vec2 v2 = vec2(p1.x, p1.y);
  Vec2 v3 = vec2(p1.x, p0.y);
  renderer_draw_line(renderer, v0, v1, color);
  renderer_draw_line(renderer, v1, v2, color);
  renderer_draw_line(renderer, v2, v3, color);
  renderer_draw_line(renderer, v3, v0, color);
}

void renderer_destroy(Renderer* renderer) {
  image_destroy(renderer->image);
  free(renderer);
}
