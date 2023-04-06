#include "config.h"
#include "engine.h"
#include "window.h"
#include <stdio.h>

Window* window;

void draw(Renderer* renderer, Player* player, DoomMap* map) {
  uint32_t width = window->width / 2;
  uint32_t height = window->height / 2;

  renderer_clear(renderer);
  for (size_t i = 0; i < map->numlinedefs; ++i) {
    LineDef linedef = map->linedefs[i];
    Vec2 v1 = map->vertices[linedef.start_vertex];
    Vec2 v2 = map->vertices[linedef.end_vertex];
    v1 = vec2_remap(v1, map->min_pos, map->max_pos, VEC2_ZERO,
                    vec2(width - 1, height - 1));
    v2 = vec2_remap(v2, map->min_pos, map->max_pos, VEC2_ZERO,
                    vec2(width - 1, height - 1));
    renderer_draw_line(renderer, v1, v2, 0x00ffffff);
  }

  Vec2 pos = vec2_remap(player->pos, map->min_pos, map->max_pos, VEC2_ZERO,
                        vec2(width - 1, height - 1));
  renderer_draw_point(renderer, pos, 0xff0000ff);
}

int main() {
  window = window_init(WIDTH, HEIGHT);
  Engine* engine = engine_init("../assets/DOOM.WAD", WIDTH / 2, HEIGHT / 2);

  while (window_is_running(window)) {
    engine_tick(engine, draw);
  }

  engine_destroy(engine);
  window_destroy(window);
}
