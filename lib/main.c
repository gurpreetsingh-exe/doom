#include "image.h"
#include "map.h"
#include "utils.h"
#include "wad.h"
#include "window.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint32_t convert_to_rgba(float x, float y, float z, float w) {
  uint8_t r = (uint8_t)(x * 255.0f);
  uint8_t g = (uint8_t)(y * 255.0f);
  uint8_t b = (uint8_t)(z * 255.0f);
  uint8_t a = (uint8_t)(w * 255.0f);

  uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
  return result;
}

/// https://rosettacode.org/wiki/Bitmap/Bresenham's_line_algorithm#C
void draw_line(Image* image, int x0, int y0, int x1, int y1, uint32_t color) {
  int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = (dx > dy ? dx : -dy) / 2, e2;

  for (;;) {
    image->data[x0 + y0 * image->width] = color;
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

int main() {
  Window* window = window_init(640, 400);
  Image* image = image_init(320, 200);
  Wad* wad = load_wad("../assets/DOOM.WAD");
  DoomMap* map = load_map(wad, "E1M1");
  (void)map;

  while (window_is_running(window)) {
    window_swap_buffers(window);
    window_get_size(window);
    image_resize(image, window->width, window->height);

    memset(image->data, 0, image->width * image->height * sizeof(uint32_t));
    for (size_t i = 0; i < map->numlinedefs; ++i) {
      LineDef linedef = map->linedefs[i];
      Vec2 v1 = map->vertices[linedef.start_vertex];
      Vec2 v2 = map->vertices[linedef.end_vertex];
      draw_line(image, v1.x, v1.y, v2.x, v2.y, 0x00ffffff);
    }

    image_set_data(image);
  }

  map_destroy(map);
  wad_destroy(wad);
  image_destroy(image);
  window_destroy(window);
}
