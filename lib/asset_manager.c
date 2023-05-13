#include "asset_manager.h"
#include "wad.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void strupr(char* s) {
  for (int i = 0; i < 8; ++i) {
    if (s[i] >= 97 && s[i] <= 122) {
      s[i] = s[i] - 32;
    }
  }
}

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

  int id1 = wad_get_map_index(wad, "S_START") + 1;
  int id2 = wad_get_map_index(wad, "S_END");
  am->numsprites = (id2 - id1);
  am->sprites = malloc(sizeof(Patch*) * am->numsprites);
  for (int i = id1; i < id2; ++i) {
    lump = wad->lumps[i];
    am->sprites[i - id1] = patch_init(am, lump.name, true);
  }

  lump = wad->lumps[wad_get_map_index(wad, "PNAMES")];
  char* pnames = malloc(lump.size);
  memcpy(pnames, wad->src + lump.filepos + 4, lump.size);

  am->texture_patches = malloc(lump.size - 4);
  am->numtexture_patches = (lump.size - 4) / 8;
  for (size_t i = 0; i < lump.size - 4; i += 8) {
    char pname[9] = {0};
    memcpy(pname, pnames + i, 8);
    pname[8] = 0;
    strupr(pname);
    am->texture_patches[i / 8] = patch_init(am, pname, false);
  }

  free(pnames);

  lump = wad->lumps[wad_get_map_index(wad, "TEXTURE1")];
  TextureHeader* texture_header = malloc(sizeof(TextureHeader));
  memcpy(texture_header, wad->src + lump.filepos, 8);
  texture_header->mtexture =
      malloc(sizeof(int32_t) * texture_header->numtextures);
  memcpy(texture_header->mtexture, wad->src + lump.filepos + 4,
         texture_header->numtextures * 4);

  am->numtexture_maps = texture_header->numtextures;
  am->texture_maps = malloc(sizeof(TextureMap*) * texture_header->numtextures);

  id1 = wad_get_map_index(wad, "F_START") + 1;
  id2 = wad_get_map_index(wad, "F_END");
  int numflats = id2 - id1;
  am->texture =
      malloc(sizeof(Texture) * (texture_header->numtextures + numflats));
  for (size_t i = 0; i < texture_header->numtextures; ++i) {
    TextureMap* tex_map = malloc(sizeof(TextureMap));
    int off = lump.filepos + texture_header->mtexture[i];
    memcpy(tex_map, wad->src + off, 22);
    tex_map->patches = malloc(sizeof(PatchMap) * tex_map->patch_count);
    for (size_t j = 0; j < tex_map->patch_count; ++j) {
      PatchMap* patch_map = &tex_map->patches[j];
      memcpy(patch_map, wad->src + off + 22 + j * 10, sizeof(PatchMap));
    }

    am->texture_maps[i] = tex_map;

    Texture* tex = &am->texture[i];
    memcpy(tex->name, tex_map->name, 8);
    tex->name[8] = 0;
    tex->width = tex_map->width;
    tex->height = tex_map->height;
    tex->image = malloc(tex_map->width * tex_map->height * sizeof(uint32_t));
    for (size_t j = 0; j < tex_map->patch_count; ++j) {
      PatchMap* patch_map = &tex_map->patches[j];
      Patch* patch = am->texture_patches[patch_map->patch];
      for (int x = 0; x < patch->width; ++x) {
        for (int y = 0; y < patch->height; ++y) {
          int x_off = x + patch_map->originx;
          int y_off = y + patch_map->originy;
          if (x_off < 0 || y_off < 0) {
            continue;
          }
          int index = x + y * patch->width;
          uint32_t color = patch->image[index];
          index = x_off + y_off * tex_map->width;
          if (index >= tex_map->width * tex_map->height) {
            continue;
          }
          tex->image[index] = color;
        }
      }
    }
    free(tex_map->patches);
    free(tex_map);

    glCreateTextures(GL_TEXTURE_2D, 1, &tex->tex);
    glBindTexture(GL_TEXTURE_2D, tex->tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex->width, tex->height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, tex->image);
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  for (int i = 0, j = texture_header->numtextures; i < numflats; ++i, ++j) {
    Texture* tex = &am->texture[j];
    lump = wad->lumps[i + id1];
    tex->image = malloc(sizeof(uint32_t) * 64 * 64);
    tex->width = 64;
    tex->height = 64;
    memcpy(tex->name, lump.name, 8);
    for (int k = 0; k < lump.size; ++k) {
      int col = *(am->wad->src + lump.filepos + k);
      int idx = col * 3;
      uint8_t* c = am->palette + idx;
      uint32_t color = 255 << 24 | c[2] << 16 | c[1] << 8 | c[0];
      tex->image[k] = color;
    }

    glCreateTextures(GL_TEXTURE_2D, 1, &tex->tex);
    glBindTexture(GL_TEXTURE_2D, tex->tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex->width, tex->height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, tex->image);
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  am->numtexture_maps = texture_header->numtextures + numflats;

  free(texture_header->mtexture);
  free(texture_header);

  return am;
}

void am_destroy(AssetManager* am) {
  for (size_t i = 0; i < am->numsprites; ++i) {
    patch_destroy(am->sprites[i]);
  }
  for (size_t i = 0; i < am->numtexture_patches; ++i) {
    patch_destroy(am->texture_patches[i]);
  }
  free(am->sprites);
  free(am->texture_patches);
  for (size_t i = 0; i < am->numtexture_maps; ++i) {
    free(am->texture[i].image);
  }
  free(am->texture_maps);
  free(am->texture);
  free(am->palette);
  glDeleteTextures(1, &am->plt);
  free(am);
}

Patch* patch_init(AssetManager* am, char* name, bool is_sprite) {
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
    free(pc->data);
  }

  if (is_sprite) {
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

    // void* temp2 = patch->image;
    // WARN: freeing messes up the texture
    // free(patch->image);
    patch->image = temp;
    patch->width = new_width;
    patch->height = new_height;
    // free(temp2);
  }

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

void patch_destroy(Patch* patch) {
  free(patch->pheader->column_offset);
  free(patch->pheader);
  for (size_t i = 0; i < patch->numcols; ++i) {
    free(patch->pcols[i]);
  }
  free(patch->pcols);
  free(patch->image);
  glDeleteTextures(1, &patch->tex);
  free(patch);
}
