#ifndef MAP_H
#define MAP_H

#include "vec2.h"
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

  Vec2 min_pos;
  Vec2 max_pos;
} DoomMap;

DoomMap* load_map(Wad* wad, char* name);
void map_calc_bounds(DoomMap* map);
void map_destroy(DoomMap* map);

#endif // !MAP_H
