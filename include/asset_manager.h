#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include "khash.h"
#include "map.h"
#include <stdint.h>

typedef struct {
  char name[9];
  PatchHeader* pheader;
  PatchColumn** pcols;
  uint32_t numcols;
  uint32_t width, height;
  uint32_t* image;
  uint32_t tex;
} Patch;

typedef struct {
  uint8_t* palette;
  Wad* wad;
  Patch** sprites;
  uint32_t plt;
} AssetManager;

AssetManager* am_init(Wad* wad, DoomMap* map);
void am_destroy(AssetManager* am);

Patch* patch_init(AssetManager* am, char* name);

#endif // !ASSET_MANAGER_H
