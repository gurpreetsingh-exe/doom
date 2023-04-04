#ifndef WAD_H
#define WAD_H

#include <stdint.h>
#include <stdlib.h>

#define THINGS 1
#define LINEDEFS 2
#define SIDEDEFS 3
#define VERTEX 4
#define SEGS 5
#define SSECTORS 6
#define NODES 7
#define SECTORS 8
#define REJECT 9
#define BLOCKMAP 10

#define WAD_VERTEX(index) wad->lumps[index + VERTEX]
#define READ_LUMP(T, dst, lump)                                                \
  dst = (T*)malloc(sizeof(T) * lump.size);                                     \
  memcpy(dst, wad->src + lump.filepos, sizeof(T) * lump.size);

typedef struct {
  int16_t x, y;
} Vec2;

typedef struct {
  uint32_t filepos;
  uint32_t size;
  char name[8];
} Lump;

typedef struct {
  char identification[4];
  int32_t numlumps;
  int32_t infotableofs;
} WadInfo;

typedef struct {
  char* src;
  WadInfo wadinfo;
  Lump* lumps;
} Wad;

typedef struct {
  char* name;
  uint32_t lump_index;
  Vec2* vertices;
} DoomMap;

Wad* load_wad(const char* path);
DoomMap* load_map(Wad* wad, char* name);
size_t wad_get_map_index(Wad* wad, const char* map_name);

#endif // !WAD_H
