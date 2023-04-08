#include "utils.h"
#include <math.h>

uint32_t convert_to_rgba(float x, float y, float z, float w) {
  uint8_t r = (uint8_t)(x * 255.0f);
  uint8_t g = (uint8_t)(y * 255.0f);
  uint8_t b = (uint8_t)(z * 255.0f);
  uint8_t a = (uint8_t)(w * 255.0f);

  uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
  return result;
}

int norm_angle(int angle) {
  angle %= 360;
  if (angle < 0) {
    angle += 360;
  }
  return angle;
}

int angle_to_screen(float angle) {
  int x = 0;
  if (angle > 90) {
    angle -= 90;
    x = 160 - round(tanf(RADIANS(angle)) * 160);
  } else {
    angle = 90 - angle;
    x = round(tanf(RADIANS(angle)) * 160);
    x += 160;
  }
  return x;
}
