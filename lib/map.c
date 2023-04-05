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

  return map;
}

void remap_vertices(DoomMap* map, uint32_t width, uint32_t height) {
  Vec2 max = {.x = -10000, .y = -10000};
  Vec2 min = {0};
  for (size_t i = 0; i < map->numvertices; ++i) {
    Vec2 v = map->vertices[i];
    max.x = MAX(v.x, max.x);
    max.y = MAX(v.y, max.y);

    min.x = MIN(v.x, min.x);
    min.y = MIN(v.y, min.y);
  }
  for (size_t i = 0; i < map->numvertices; ++i) {
    Vec2* v = &map->vertices[i];
    v->x = MAP_RANGE((float)v->x, min.x, max.x, 0, width - 1);
    v->y = MAP_RANGE((float)v->y, min.y, max.y, 0, height - 1);
  }
}

void map_destroy(DoomMap* map) {
  free(map->things);
  free(map->linedefs);
  free(map->vertices);
  free(map->segments);
  free(map->subsectors);
  free(map->nodes);
  free(map);
}
