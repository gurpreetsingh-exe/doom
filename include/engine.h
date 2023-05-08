#ifndef ENGINE_H
#define ENGINE_H

#include "asset_manager.h"
#include "map.h"
#include "map_renderer.h"
#include "player.h"
#include "renderer.h"
#include "view_renderer.h"
#include "wad.h"

typedef struct {
  Renderer* renderer;
  DoomMap* map;
  Player* player;
  Wad* wad;
  MapRenderer* map_renderer;
  ViewRenderer* vr;
  AssetManager* asset_manager;
  uint32_t width, height;
} Engine;

Engine* engine_init(const char* wad_path, uint32_t width, uint32_t height);
void engine_tick(Engine* engine, void (*draw)(Engine*),
                 void (*update)(Engine*, Player*, Event*));
void engine_destroy(Engine* engine);

#endif // !ENGINE_H
