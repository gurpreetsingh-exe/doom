#include "config.h"
#include "image.h"
#include "map.h"
#include "renderer.h"
#include "utils.h"
#include "wad.h"
#include "window.h"
#include <stdio.h>

int main() {
  Window* window = window_init(WIDTH, HEIGHT);
  Image* image = image_init(WIDTH / 2, HEIGHT / 2);
  Wad* wad = load_wad("../assets/DOOM.WAD");
  DoomMap* map = load_map(wad, "E1M1");
  remap_vertices(map, image->width, image->height);
  Renderer* renderer = renderer_init(image);

  while (window_is_running(window)) {
    window_swap_buffers(window);
    window_get_size(window);
    image_resize(image, window->width, window->height);

    renderer_clear(renderer);
    for (size_t i = 0; i < map->numlinedefs; ++i) {
      LineDef linedef = map->linedefs[i];
      Vec2 v1 = map->vertices[linedef.start_vertex];
      Vec2 v2 = map->vertices[linedef.end_vertex];
      renderer_draw_line(renderer, v1, v2, 0x00ffffff);
    }

    image_set_data(image);
  }

  map_destroy(map);
  wad_destroy(wad);
  renderer_destroy(renderer);
  window_destroy(window);
}
