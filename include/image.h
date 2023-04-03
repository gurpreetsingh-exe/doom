#ifndef IMAGE_H
#define IMAGE_H

// clang-format off
#include <GL/glew.h>
// clang-format on
#include <stdint.h>

typedef struct {
  uint32_t id;
  uint32_t framebuffer;
  uint32_t* data;
  uint32_t width, height;
} Image;

Image* image_init(uint32_t, uint32_t);
void image_set_data(Image*);
void image_resize(Image*, uint32_t, uint32_t);
void image_destroy(Image*);

#endif // !IMAGE_H
