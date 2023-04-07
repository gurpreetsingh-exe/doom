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
