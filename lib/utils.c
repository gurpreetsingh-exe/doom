#include "utils.h"
#include "config.h"
#include "window.h"
#include <math.h>
#include <stdio.h>

extern Window* window;

uint32_t convert_to_rgba(float x, float y, float z, float w) {
  uint8_t r = (uint8_t)(x * 255.0f);
  uint8_t g = (uint8_t)(y * 255.0f);
  uint8_t b = (uint8_t)(z * 255.0f);
  uint8_t a = (uint8_t)(w * 255.0f);

  uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
  return result;
}

float norm_angle(float angle) {
  angle = fmodf(angle, 360);
  if (angle < 0) {
    angle += 360;
  }
  return angle;
}

int angle_to_screen(float angle) {
  int hw = window->width / 2;
  float screen_dist = hw;
  float x = 0;
#if 1
  if (angle > 0) {
    x = screen_dist - tanf(RADIANS(angle)) * hw;
  } else {
    x = -tanf(RADIANS(angle)) * hw + screen_dist;
  }
#else
  if (angle > 90) {
    angle -= 90;
    x = screen_dist - round(tan(RADIANS(angle) * hw));
  } else {
    angle = 90 - angle;
    x = round(tan(RADIANS(angle)) * hw) + screen_dist;
  }
#endif
  return x;
}
