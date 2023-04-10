#include "map.h"
#include "config.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

DoomMap* load_map(Wad* wad, char* name) {
  size_t index = wad_get_map_index(wad, name);
  DoomMap* map = (DoomMap*)malloc(sizeof(DoomMap));
  map->lump_index = index;

  Lump lump = WAD_THING(index);
  READ_LUMP(Thing, map->things, lump);
  map->numthings = lump.size / sizeof(Thing);

  lump = WAD_LINEDEF(index);
  READ_LUMP(LineDef, map->linedefs, lump);
  map->numlinedefs = lump.size / sizeof(LineDef);

  lump = WAD_SIDEDEF(index);
  READ_LUMP(SideDef, map->sidedefs, lump);
  map->numsidedefs = lump.size / sizeof(SideDef);

  lump = WAD_VERTEX(index);
  READ_LUMP(Vec2, map->vertices, lump);
  map->numvertices = lump.size / sizeof(Vec2);

  lump = WAD_SEG(index);
  READ_LUMP(Segment, map->segments, lump);
  map->numsegments = lump.size / sizeof(Segment);

  lump = WAD_SSECTOR(index);
  READ_LUMP(SubSector, map->subsectors, lump);
  map->numsubsectors = lump.size / sizeof(SubSector);

  lump = WAD_NODE(index);
  READ_LUMP(Node, map->nodes, lump);
  map->numnodes = lump.size / sizeof(Node);

  lump = WAD_SECTOR(index);
  READ_LUMP(Sector, map->sectors, lump);
  map->numsectors = lump.size / sizeof(Sector);

  map_calc_bounds(map);

  return map;
}

void map_calc_bounds(DoomMap* map) {
  map->max_pos = (Vec2){.x = -10000, .y = -10000};
  map->min_pos = (Vec2){0};
  for (size_t i = 0; i < map->numvertices; ++i) {
    Vec2 v = map->vertices[i];
    map->max_pos = vec2_max(v, map->max_pos);
    map->min_pos = vec2_min(v, map->min_pos);
  }
}

void map_destroy(DoomMap* map) {
  free(map->things);
  free(map->linedefs);
  free(map->sidedefs);
  free(map->vertices);
  free(map->segments);
  free(map->subsectors);
  free(map->nodes);
  free(map->sectors);
  free(map);
}
