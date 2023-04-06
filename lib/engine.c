#include "engine.h"
#include "window.h"

extern Window* window;

Engine* engine_init(const char* wad_path, uint32_t width, uint32_t height) {
  Engine* engine = (Engine*)malloc(sizeof(Engine));
  engine->width = width;
  engine->height = height;
  engine->wad = load_wad(wad_path);
  engine->map = load_map(engine->wad, "E1M1");
  engine->player = player_init(engine->map->things[0]);
  Image* image = image_init(width, height);
  engine->renderer = renderer_init(image);
  return engine;
}

void engine_tick(Engine* engine, void (*draw)(Renderer*, Player*, DoomMap*)) {
  window_get_size(window);
  renderer_resize(engine->renderer, window->width, window->height);
  draw(engine->renderer, engine->player, engine->map);
  renderer_submit(engine->renderer);
}

void engine_destroy(Engine* engine) {
  player_destroy(engine->player);
  map_destroy(engine->map);
  wad_destroy(engine->wad);
  renderer_destroy(engine->renderer);
  free(engine);
}
