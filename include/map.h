#ifndef MAP_H
#define MAP_H

#include "wad.h"
#include <stdint.h>

typedef struct {
  char* name;
  uint32_t lump_index;
  Thing* things;
  uint32_t numthings;
  LineDef* linedefs;
  uint32_t numlinedefs;
  Vec2* vertices;
  uint32_t numvertices;
  Segment* segments;
  uint32_t numsegments;
  SubSector* subsectors;
  uint32_t numsubsectors;
  Node* nodes;
  uint32_t numnodes;
} DoomMap;

DoomMap* load_map(Wad* wad, char* name);
void remap_vertices(DoomMap* map, uint32_t width, uint32_t height);
void map_destroy(DoomMap* map);

#endif // !MAP_H
