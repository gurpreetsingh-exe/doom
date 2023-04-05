#include "image.h"
#include <stdlib.h>

Image* image_init(uint32_t width, uint32_t height) {
  Image* image = (Image*)malloc(sizeof(Image));
  image->width = width;
  image->height = height;
  image->upscale = 2;
  image->data = (uint32_t*)calloc(sizeof(uint32_t), width * height);

  glGenTextures(1, &image->id);
  glBindTexture(GL_TEXTURE_2D, image->id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glGenFramebuffers(1, &image->framebuffer);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, image->framebuffer);
  glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_2D, image->id, 0);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

  return image;
}

void image_set_data(Image* image) {
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->width, image->height, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, image->data);

  glBindFramebuffer(GL_READ_FRAMEBUFFER, image->framebuffer);
  glBlitFramebuffer(
      0, 0, image->width, image->height, 0, 0, image->width * image->upscale,
      image->height * image->upscale, GL_COLOR_BUFFER_BIT, GL_NEAREST);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void image_resize(Image* image, uint32_t width, uint32_t height) {
  if (image->width * image->upscale == width &&
      image->height * image->upscale == height) {
    return;
  }

  image->width = width / image->upscale;
  image->height = height / image->upscale;
  image->data = (uint32_t*)realloc(
      image->data, sizeof(uint32_t) * image->width * image->height);
}

void image_destroy(Image* image) {
  glDeleteTextures(1, &image->id);
  glDeleteFramebuffers(1, &image->framebuffer);
  free(image->data);
  free(image);
}
