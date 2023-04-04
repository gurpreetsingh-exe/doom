#include "image.h"
#include "wad.h"
#include "window.h"
#include <stdio.h>

uint32_t convert_to_rgba(float x, float y, float z, float w) {
  uint8_t r = (uint8_t)(x * 255.0f);
  uint8_t g = (uint8_t)(y * 255.0f);
  uint8_t b = (uint8_t)(z * 255.0f);
  uint8_t a = (uint8_t)(w * 255.0f);

  uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
  return result;
}

int main() {
  Window* window = window_init(640, 400);
  Image* image = image_init(640, 400);
  Wad* wad = load_wad("../assets/DOOM.WAD");
  DoomMap* map = load_map(wad, "E1M1");
  (void)map;

  while (window_is_running(window)) {
    window_swap_buffers(window);
    window_get_size(window);
    image_resize(image, window->width, window->height);

    uint32_t width = image->width;
    uint32_t height = image->height;
    for (uint32_t i = 0; i < width; ++i) {
      for (uint32_t j = 0; j < height; ++j) {
        image->data[i + j * width] =
            convert_to_rgba((float)i / width, (float)j / height, 1.0, 1.0);
      }
    }

    image_set_data(image);
  }

  map_destroy(map);
  wad_destroy(wad);
  image_destroy(image);
  window_destroy(window);
}
