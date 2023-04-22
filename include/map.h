#ifndef MAP_H
#define MAP_H

#include "player.h"
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
  SideDef* sidedefs;
  uint32_t numsidedefs;
  Vec2* vertices;
  uint32_t numvertices;
  Segment* segments;
  uint32_t numsegments;
  SubSector* subsectors;
  uint32_t numsubsectors;
  Node* nodes;
  uint32_t numnodes;
  Sector* sectors;
  uint32_t numsectors;

  LineDef_t* linedefs_t;
  SideDef_t* sidedefs_t;
  Segment_t* segments_t;

  Vec2 min_pos;
  Vec2 max_pos;
} DoomMap;

DoomMap* load_map(Wad* wad, char* name);
void map_calc_bounds(DoomMap* map);
int map_get_ssector_height(DoomMap* map, Player* player);
void map_destroy(DoomMap* map);

#endif // !MAP_H
