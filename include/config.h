#ifndef CONFIG_H
#define CONFIG_H

#define WIDTH 640
#define HEIGHT 400
#define FOV 90
#define HALF_FOV 45
#define EYE_LEVEL 41

#include <stdbool.h>

typedef struct {
  bool debug_bsp_view;
  bool display_map;
  bool debug_fov;
  bool debug_sectors;
  bool top_view;
  bool v_sync;
  bool clip_view;
  int segments;
} Config;

#endif // !CONFIG_H
