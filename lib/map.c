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

  map->sidedefs_t = malloc(sizeof(SideDef_t) * map->numsidedefs);
  for (int i = 0; i < map->numsidedefs; ++i) {
    SideDef_t* sd = &map->sidedefs_t[i];
    memcpy(sd, &map->sidedefs[i], sizeof(SideDef));
    sd->sector = &map->sectors[sd->sector_num];
  }

  map->linedefs_t = malloc(sizeof(LineDef_t) * map->numlinedefs);
  for (int i = 0; i < map->numlinedefs; ++i) {
    LineDef_t* ld = &map->linedefs_t[i];
    memcpy(ld, &map->linedefs[i], sizeof(LineDef));
    ld->front_sidedef = &map->sidedefs_t[ld->front_sidedef_id];
    if (ld->back_sidedef_id == -1) {
      ld->back_sidedef = NULL;
    } else {
      ld->back_sidedef = &map->sidedefs_t[ld->back_sidedef_id];
    }
  }

  map->segments_t = malloc(sizeof(Segment_t) * map->numsegments);
  for (int i = 0; i < map->numsegments; ++i) {
    Segment_t* seg = &map->segments_t[i];
    memcpy(seg, &map->segments[i], sizeof(Segment));
    seg->start_vertex = &map->vertices[seg->start_vertex_id];
    seg->end_vertex = &map->vertices[seg->end_vertex_id];
    seg->linedef = &map->linedefs_t[seg->linedef_id];
    SideDef_t* front;
    SideDef_t* back;
    if (seg->direction) {
      front = seg->linedef->back_sidedef;
      back = seg->linedef->front_sidedef;
    } else {
      front = seg->linedef->front_sidedef;
      back = seg->linedef->back_sidedef;
    }
    seg->front_sector = front->sector;
    if (4 & seg->linedef->flags) {
      seg->back_sector = back->sector;
    } else {
      seg->back_sector = NULL;
    }
    seg->angle = (seg->angle << 16) * 8.38190317e-8;
    seg->angle = seg->angle < 0 ? seg->angle + 360 : seg->angle;
  }

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

int map_get_ssector_height(DoomMap* map, Player* player) {
  int16_t subsec_id = map->numnodes - 1;
  while (!(subsec_id & SSECTOR_IDENTIFIER)) {
    Node* node = &map->nodes[subsec_id];
    if (player_is_on_side(player, node)) {
      subsec_id = node->left_child;
    } else {
      subsec_id = node->right_child;
    }
  }
  subsec_id &= ~SSECTOR_IDENTIFIER;
  SubSector subsector = map->subsectors[subsec_id];
  Segment_t seg = map->segments_t[subsector.first_seg];
  return seg.front_sector->floor_height;
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
