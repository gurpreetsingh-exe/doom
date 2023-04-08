#ifndef CONFIG_H
#define CONFIG_H

#define WIDTH 640
#define HEIGHT 400
#define FOV 90
#define HALF_FOV 45

#include <stdbool.h>

typedef struct {
  bool debug_bsp_view;
  bool display_map;
  bool debug_fov;
  bool debug_sectors;
} Config;

#endif // !CONFIG_H
