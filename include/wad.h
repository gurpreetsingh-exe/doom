#ifndef WAD_H
#define WAD_H

#include "vec2.h"
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

#define SSECTOR_IDENTIFIER 0x8000

#define WAD_THING(index) wad->lumps[index + THINGS]
#define WAD_LINEDEF(index) wad->lumps[index + LINEDEFS]
#define WAD_SIDEDEF(index) wad->lumps[index + SIDEDEFS]
#define WAD_VERTEX(index) wad->lumps[index + VERTEX]
#define WAD_SEG(index) wad->lumps[index + SEGS]
#define WAD_SSECTOR(index) wad->lumps[index + SSECTORS]
#define WAD_NODE(index) wad->lumps[index + NODES]
#define WAD_SECTOR(index) wad->lumps[index + SECTORS]
#define WAD_REJECT(index) wad->lumps[index + REJECT]
#define WAD_BLOCKMAP(index) wad->lumps[index + BLOCKMAP]

#define READ_LUMP(T, dst, lump)                                                \
  dst = (T*)malloc(lump.size);                                                 \
  memcpy(dst, wad->src + lump.filepos, lump.size);

typedef struct {
  int16_t x;
  int16_t y;
  int16_t angle;
  int16_t type;
  int16_t flags;
} Thing;

typedef struct {
  int16_t start_vertex;
  int16_t end_vertex;
  int16_t flags;
  int16_t special_type;
  int16_t sector_tag;
  int16_t front_sidedef;
  int16_t back_sidedef;
} LineDef;

typedef struct {
  int16_t x_offset;
  int16_t y_offset;
  char upper_texture[8];
  char lower_texture[8];
  char middle_texture[8];
  int16_t sector_num;
} SideDef;

typedef struct {
  int16_t seg_count;
  int16_t first_seg;
} SubSector;

typedef struct {
  int16_t start_vertex;
  int16_t end_vertex;
  int16_t angle;
  int16_t linedef;
  int16_t direction; // 0 (same as linedef) or 1 (opposite of linedef)
  int16_t offset;    // distance along linedef to start of seg
} Segment;

typedef struct {
  int16_t top;
  int16_t bottom;
  int16_t left;
  int16_t right;
} BBox;

typedef struct {
  int16_t x_partition;
  int16_t y_partition;
  int16_t dx_partition;
  int16_t dy_partition;
  BBox right;
  BBox left;
  int16_t right_child;
  int16_t left_child;
} Node;

typedef struct {
  int16_t floor_height;
  int16_t ceiling_height;
  char floor_texture[8];
  char ceiling_texture[8];
  int16_t light_level;
  int16_t special_type;
  int16_t tag_num;
} Sector;

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
  int16_t x_offset;
  int16_t y_offset;
  char upper_texture[8];
  char lower_texture[8];
  char middle_texture[8];
  int16_t sector_num;

  Sector* sector;
} SideDef_t;

typedef struct {
  int16_t start_vertex;
  int16_t end_vertex;
  int16_t flags;
  int16_t special_type;
  int16_t sector_tag;
  int16_t front_sidedef_id;
  int16_t back_sidedef_id;

  SideDef_t* front_sidedef;
  SideDef_t* back_sidedef;
} LineDef_t;

typedef struct {
  int16_t start_vertex_id;
  int16_t end_vertex_id;
  int16_t angle;
  int16_t linedef_id;
  int16_t direction; // 0 (same as linedef) or 1 (opposite of linedef)
  int16_t offset;    // distance along linedef to start of seg

  Vec2* start_vertex;
  Vec2* end_vertex;
  LineDef_t* linedef;
  Sector* front_sector;
  Sector* back_sector;
} Segment_t;

Wad* load_wad(const char* path);
size_t wad_get_map_index(Wad* wad, const char* map_name);
void wad_destroy(Wad* wad);

#endif // !WAD_H
