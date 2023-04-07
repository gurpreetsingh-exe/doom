#include "config.h"
#include "engine.h"
#include "map_renderer.h"

Window* window;

void draw(MapRenderer* map_renderer) {
  DoomMap* map = map_renderer->map;
  Player* player = map_renderer->player;

  map_renderer_draw_map(map_renderer);
  map_renderer_draw_node(map_renderer, map->numnodes - 1);
  Vec2 pos = vec2_remap_window(player->pos, map->min_pos, map->max_pos);
  renderer_draw_point(map_renderer->renderer, pos, 0xff0000ff);
}

int main() {
  window = window_init(WIDTH, HEIGHT);
  Engine* engine = engine_init("../assets/DOOM.WAD", WIDTH / 2, HEIGHT / 2);

  while (window_is_running(window)) {
    renderer_clear(engine->renderer);
    engine_tick(engine, draw);
  }

  engine_destroy(engine);
  window_destroy(window);
}
