#include "renderer.h"
#include "image.h"
#include <stdlib.h>
#include <string.h>

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
    renderer->image->data[x0 + y0 * renderer->image->width] = color;
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

void renderer_clear(Renderer* renderer) {
  memset(renderer->image->data, 0,
         renderer->image->width * renderer->image->height * sizeof(uint32_t));
}

void renderer_destroy(Renderer* renderer) {
  image_destroy(renderer->image);
  free(renderer);
}
