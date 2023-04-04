#include "wad.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

Wad* load_wad(const char* path) {
  Wad* wad = (Wad*)malloc(sizeof(Wad));
  FILE* file = fopen(path, "r");
  if (!file) {
    fprintf(stderr, "%s: failed to open file %s\n", strerror(errno), path);
    exit(1);
  }
  fseek(file, 0L, SEEK_END);
  int64_t size = ftell(file);
  rewind(file);

  char* data = malloc(size);
  fread(data, 1, size, file);
  fclose(file);

  memcpy(&wad->wadinfo, data, 12);
  if (memcmp("IWAD", wad->wadinfo.identification, 4)) {
    printf("invalid wad format identifier\n");
    exit(1);
  }

  wad->lumps = (Lump*)malloc(sizeof(Lump) * wad->wadinfo.numlumps);
  memcpy(wad->lumps, data + wad->wadinfo.infotableofs,
         sizeof(Lump) * wad->wadinfo.numlumps);

  wad->src = data;
  return wad;
}

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

size_t wad_get_map_index(Wad* wad, const char* map_name) {
  for (size_t i = 0; i < wad->wadinfo.numlumps; ++i) {
    if (strncmp(map_name, wad->lumps[i].name, 8) == 0) {
      return i;
    }
  }
  return -1;
}

void wad_destroy(Wad* wad) {
  free(wad->src);
  free(wad->lumps);
  free(wad);
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
