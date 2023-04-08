#include "config.h"
#include "engine.h"
#include "imgui_layer.h"
#include "map_renderer.h"
#include <math.h>
#include <stdio.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

Window* window;

void draw(MapRenderer* map_renderer) {
  DoomMap* map = map_renderer->map;
  Player* player = map_renderer->player;
  map_renderer_draw_map(map_renderer);

  if (map_renderer->config->debug_fov) {
    Vec2 pos = vec2_remap_window(player->pos, map->min_pos, map->max_pos);
    float angle = RADIANS(-player->angle + 45);
    float sin_a1 = sin((angle - PI / 4));
    float cos_a1 = cos((angle - PI / 4));
    float sin_a2 = sin((angle + PI / 4));
    float cos_a2 = cos((angle + PI / 4));
    int ray_len = 30;

    Vec2 v0 = vec2_add(pos, vec2(ray_len * sin_a1, ray_len * cos_a1));
    Vec2 v1 = vec2_add(pos, vec2(ray_len * sin_a2, ray_len * cos_a2));
    renderer_draw_line(map_renderer->renderer, pos, v0, 0xff00ffff);
    renderer_draw_line(map_renderer->renderer, pos, v1, 0xff00ffff);
    renderer_draw_point(map_renderer->renderer, pos, 0xff0000ff);
  }
}

void update(Player* player, Event* event) {
  int speed = event->delta_time * 0.4;
  float rot_speed = DEGREES(event->delta_time * 0.004);

  if (event->pressed[GLFW_KEY_LEFT]) {
    player->angle += rot_speed;
  }
  if (event->pressed[GLFW_KEY_RIGHT]) {
    player->angle -= rot_speed;
  }

  float angle = RADIANS(player->angle - 45);
  Vec2 inc = VEC2_ZERO;
  if (event->pressed[GLFW_KEY_D]) {
    inc = rotate(vec2(speed, 0), angle);
  }
  if (event->pressed[GLFW_KEY_A]) {
    inc = rotate(vec2(-speed, 0), angle);
  }
  if (event->pressed[GLFW_KEY_W]) {
    inc = rotate(vec2(0, speed), angle);
  }
  if (event->pressed[GLFW_KEY_S]) {
    inc = rotate(vec2(0, -speed), angle);
  }
  player->pos = vec2_add(player->pos, inc);
}

int main() {
  Config config = {
      .debug_bsp_view = false,
      .display_map = false,
      .debug_fov = true,
      .debug_sectors = true,
  };

  window = window_init(WIDTH, HEIGHT);
  Engine* engine =
      engine_init("../assets/DOOM.WAD", WIDTH / 2, HEIGHT / 2, &config);
  imgui_init();

  ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
  while (window_is_running(window)) {
    imgui_begin_frame();
    igBegin("Debug", NULL, window_flags);
    igCheckbox("BSP Nodes", &config.debug_bsp_view);
    igCheckbox("Map", &config.display_map);
    igCheckbox("Debug FOV", &config.debug_fov);
    igCheckbox("Sectors", &config.debug_sectors);
    igEnd();
    renderer_clear(engine->renderer);
    engine_tick(engine, draw, update);
    imgui_end_frame();
  }

  imgui_destroy();
  engine_destroy(engine);
  window_destroy(window);
}
