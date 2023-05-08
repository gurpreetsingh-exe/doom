#include "asset_manager.h"
#include "wad.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int hash(char* name);
#define SCALE 2

AssetManager* am_init(Wad* wad, DoomMap* map) {
  AssetManager* am = malloc(sizeof(AssetManager));
  am->wad = wad;
  Lump lump = wad->lumps[wad_get_map_index(wad, "PLAYPAL")];
  am->palette = malloc(lump.size);
  memcpy(am->palette, wad->src + lump.filepos, lump.size);

  uint32_t plt = 0;
  glCreateTextures(GL_TEXTURE_2D, 1, &plt);
  glBindTexture(GL_TEXTURE_2D, plt);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 16, 16, 0, GL_RGB, GL_UNSIGNED_BYTE,
               am->palette);
  glBindTexture(GL_TEXTURE_2D, 0);
  am->plt = plt;

  // khash_t(s)* sprites = kh_init(s);

  int id1 = wad_get_map_index(wad, "S_START") + 1;
  int id2 = wad_get_map_index(wad, "S_END");
  am->sprites = malloc(sizeof(Patch*) * (id2 - id1));
  printf("%d\n", id2 - id1);
  for (int i = id1; i < id2; ++i) {
    lump = wad->lumps[i];
    am->sprites[i - id1] = patch_init(am, lump.name);
    // k = kh_put(s, sprites, key, &ret);
    // p = patch_init(am, key);
    // kh_value(sprites, k) = p;
  }

  return am;
}

void am_destroy(AssetManager* am) { free(am); }

Patch* patch_init(AssetManager* am, char* name) {
  Patch* patch = malloc(sizeof(Patch));
  memcpy(patch->name, name, 8);
  patch->name[8] = 0;
  int lump_index = wad_get_map_index(am->wad, name);
  Lump lump = am->wad->lumps[lump_index];

  PatchHeader* ph = malloc(sizeof(PatchHeader));
  memcpy(ph, am->wad->src + lump.filepos, 8);

  ph->column_offset = malloc(sizeof(uint32_t) * ph->width);
  memcpy(ph->column_offset, am->wad->src + lump.filepos + 8,
         sizeof(uint32_t) * ph->width);

  patch->pheader = ph;
  patch->width = ph->width;
  patch->height = ph->height;

  int cap = 4;
  patch->pcols = malloc(sizeof(PatchColumn*) * cap);
  patch->numcols = 0;
  for (size_t i = 0; i < ph->width; ++i) {
    int off = lump.filepos + ph->column_offset[i];
    while (true) {
      PatchColumn* pc = malloc(sizeof(PatchColumn));
      char* src = am->wad->src + off;
      pc->top_delta = src[0];
      if (pc->top_delta != 0xff) {
        pc->length = src[1];
        pc->unused = src[2];
        pc->data = malloc(pc->length);
        memcpy(pc->data, src + 3, pc->length);
        pc->unused2 = src[3 + pc->length];
        off += 4 + pc->length;
      } else {
        off += 1;
      }
      patch->pcols[patch->numcols] = pc;
      patch->numcols += 1;
      if (patch->numcols == cap) {
        cap = cap * 1.5;
        patch->pcols = realloc(patch->pcols, sizeof(PatchColumn*) * cap);
      }
      if (pc->top_delta == 0xff) {
        break;
      }
    }
  }

  patch->image = malloc(ph->width * ph->height * sizeof(uint32_t));
  for (size_t i = 0, ix = 0; i < patch->numcols; ++i) {
    PatchColumn* pc = patch->pcols[i];
    if (pc->top_delta == 0xff) {
      ix++;
      continue;
    }
    for (size_t j = 0; j < pc->length; ++j) {
      int idx = pc->data[j] * 3;
      uint8_t* c = am->palette + idx;
      uint32_t color = 255 << 24 | c[2] << 16 | c[1] << 8 | c[0];
      int pixel = ix + (j + pc->top_delta) * ph->width;
      patch->image[pixel] = color;
    }
  }

  int new_width = ph->width * SCALE;
  int new_height = ph->height * SCALE;
  uint32_t* temp = malloc(new_width * new_height * sizeof(uint32_t));
  for (size_t i = 0; i < new_width; ++i) {
    for (size_t j = 0; j < new_height; ++j) {
      int x = floor((float)i / SCALE);
      int y = floor((float)j / SCALE);
      uint32_t color = patch->image[x + y * ph->width];
      int alpha = (color >> 24);
      uint32_t* dst = &temp[i + j * new_width];
      *dst = color;
    }
  }

  // WARN: freeing messes up the texture
  // free(patch->image);
  patch->image = temp;
  patch->width = new_width;
  patch->height = new_height;

  uint32_t tex = 0;
  glCreateTextures(GL_TEXTURE_2D, 1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, patch->width, patch->height, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, patch->image);
  glBindTexture(GL_TEXTURE_2D, 0);
  patch->tex = tex;

  return patch;
}
