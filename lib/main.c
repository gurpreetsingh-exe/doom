#include "config.h"
#include "engine.h"
#include "imgui_layer.h"
#include "map_renderer.h"
#include <math.h>
#include <stdio.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

Window* window;
Config config;

void draw(Engine* engine) {
  DoomMap* map = engine->map;
  Player* player = engine->player;
  config.segments = 0;

  if (config.top_view) {
    MapRenderer* map_renderer = engine->map_renderer;
    map_renderer_draw_map(map_renderer);

    if (config.debug_fov) {
      Vec2 pos = vec2_remap_window(player->pos, map->min_pos, map->max_pos);
      float angle = -player->angle + 90;
      float sin_a1 = sin(RADIANS(angle - HALF_FOV));
      float cos_a1 = cos(RADIANS(angle - HALF_FOV));
      float sin_a2 = sin(RADIANS(angle + HALF_FOV));
      float cos_a2 = cos(RADIANS(angle + HALF_FOV));
      int ray_len = 30;

      Vec2 v0 = vec2_add(pos, vec2(ray_len * sin_a1, ray_len * cos_a1));
      Vec2 v1 = vec2_add(pos, vec2(ray_len * sin_a2, ray_len * cos_a2));
      renderer_draw_line(map_renderer->renderer, pos, v0, 0xff00ffff);
      renderer_draw_line(map_renderer->renderer, pos, v1, 0xff00ffff);
      renderer_draw_point(map_renderer->renderer, pos, 0xff0000ff);
    }
  } else {
    ViewRenderer* vr = engine->vr;
    vr_init_frame(vr);
    vr_draw(vr);
  }
}

void update(Engine* engine, Player* player, Event* event) {
  float speed = event->delta_time * 0.4;
  float rot_speed = event->delta_time * 0.1;

  if (event->pressed[GLFW_KEY_LEFT]) {
    player->angle += rot_speed;
  }
  if (event->pressed[GLFW_KEY_RIGHT]) {
    player->angle -= rot_speed;
  }

  Vec2 inc = VEC2_ZERO;
  if (event->pressed[GLFW_KEY_W]) {
    inc = vec2(speed, 0);
  }
  if (event->pressed[GLFW_KEY_S]) {
    inc = vec2(-speed, 0);
  }
  if (event->pressed[GLFW_KEY_A]) {
    inc = vec2(0, speed);
  }
  if (event->pressed[GLFW_KEY_D]) {
    inc = vec2(0, -speed);
  }
  inc = rotate(inc, player->angle);
  player->pos = vec2_add(player->pos, inc);
  player->z = EYE_LEVEL + map_get_ssector_height(engine->map, player);
}

int main() {
  config = (Config){
      .debug_bsp_view = false,
      .display_map = false,
      .debug_fov = true,
      .debug_sectors = true,
      .top_view = true,
      .v_sync = true,
      .clip_view = true,
  };

  window = window_init(WIDTH, HEIGHT);
  Engine* engine = engine_init("../assets/DOOM.WAD", WIDTH / 2, HEIGHT / 2);
  imgui_init();
  ImGuiIO* io = igGetIO();

  ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
  int tex_id = 10;
  int tex_id2 = 0;
  int scale = 2;
  while (window_is_running(window)) {
    glfwSwapInterval(config.v_sync);
    imgui_begin_frame();
    igBegin("Debug", NULL, window_flags);
    igText("DeltaTime: %2.3f ms", 1000.0 / io->Framerate);
    igText("FPS: %d", (int)io->Framerate);
    igText("Segments: %d", config.segments);
    igCheckbox("V-Sync", &config.v_sync);
    igSeparator();
    igText("Display: %s\n", config.top_view ? "Map View" : "Player View");
    igSeparator();
    if (config.top_view) {
      igCheckbox("BSP Nodes", &config.debug_bsp_view);
      igCheckbox("Map", &config.display_map);
      igCheckbox("Debug FOV", &config.debug_fov);
      igCheckbox("Sectors", &config.debug_sectors);
      igCheckbox("Clip Segments", &config.clip_view);
    }
    igEnd();

    igBegin("Image Viewer", NULL, window_flags);
    ImGuiSliderFlags flags = ImGuiSliderFlags_None;
    igSliderInt("TexId: ", &tex_id, 0, 763, "", flags);
    igInputInt("TexInput: ", &tex_id, 1, 10, ImGuiInputFlags_None);
    igSliderInt("Scale: ", &scale, 1, 10, "", flags);
    tex_id = MIN(MAX(tex_id, 0), 763);
    Patch* patch = engine->asset_manager->sprites[tex_id];
    igText("    Name: %s\n", patch->name);
    igText("    Width: %d\n", patch->width);
    igText("    Height: %d\n", patch->height);
    igImage((void*)(intptr_t)patch->tex,
            (ImVec2){.x = patch->width * scale, .y = patch->height * scale},
            (ImVec2){.x = 0, .y = 0}, (ImVec2){.x = 1, .y = 1},
            (ImVec4){.x = 1, .y = 1, .z = 1, .w = 1},
            (ImVec4){.x = 0, .y = 0, .z = 0, .w = 0});
    igEnd();

    igBegin("Color Palette", NULL, window_flags);
    igImage((void*)(intptr_t)engine->asset_manager->plt,
            (ImVec2){.x = 128, .y = 128}, (ImVec2){.x = 0, .y = 0},
            (ImVec2){.x = 1, .y = 1}, (ImVec4){.x = 1, .y = 1, .z = 1, .w = 1},
            (ImVec4){.x = 0, .y = 0, .z = 0, .w = 0});
    igEnd();

    igBegin("Texture Viewer", NULL, window_flags);
    int sz = engine->asset_manager->numtexture_maps;
    igSliderInt("TexId: ", &tex_id2, 0, sz - 1, "", flags);
    igInputInt("TexInput: ", &tex_id2, 1, 10, ImGuiInputFlags_None);
    tex_id2 = MIN(MAX(tex_id2, 0), sz);
    Texture* tex = &engine->asset_manager->texture[tex_id2];
    igText("    Name: %s\n", tex->name);
    igText("    Width: %d\n", tex->width);
    igText("    Height: %d\n", tex->height);
    igImage((void*)(intptr_t)tex->tex,
            (ImVec2){.x = tex->width, .y = tex->height},
            (ImVec2){.x = 0, .y = 0}, (ImVec2){.x = 1, .y = 1},
            (ImVec4){.x = 1, .y = 1, .z = 1, .w = 1},
            (ImVec4){.x = 0, .y = 0, .z = 0, .w = 0});
    igEnd();

    igBegin("Color Palette", NULL, window_flags);
    igImage((void*)(intptr_t)engine->asset_manager->plt,
            (ImVec2){.x = 128, .y = 128}, (ImVec2){.x = 0, .y = 0},
            (ImVec2){.x = 1, .y = 1}, (ImVec4){.x = 1, .y = 1, .z = 1, .w = 1},
            (ImVec4){.x = 0, .y = 0, .z = 0, .w = 0});
    igEnd();

    renderer_clear(engine->renderer);
    engine_tick(engine, draw, update);
    imgui_end_frame();
  }

  imgui_destroy();
  engine_destroy(engine);
  window_destroy(window);
}
