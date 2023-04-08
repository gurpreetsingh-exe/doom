#include "utils.h"
#include "config.h"
#include <math.h>
#include <stdio.h>

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

int angle_to_screen(float angle, int hw) {
  float SCREEN_DIST = hw / tan(RADIANS(HALF_FOV));

  // if angle > 0:
  //     x = SCREEN_DIST - math.tan(math.radians(angle)) * H_WIDTH
  // else:
  //     x = -math.tan(math.radians(angle)) * H_WIDTH + SCREEN_DIST
  // return int(x)
  float x = 0;
  if (angle > 0) {
    x = SCREEN_DIST - tan(RADIANS(angle)) * hw;
    // printf("angle_to_screen: %f\n", x);
  } else {
    x = -tan(RADIANS(angle)) * hw + SCREEN_DIST;
  }
  return x;

  // int x = 0;
  // if (angle > 90) {
  //   angle -= 90;
  //   x = 160 - round(tanf(RADIANS(angle)) * 160);
  // } else {
  //   angle = 90 - angle;
  //   x = round(tanf(RADIANS(angle)) * 160);
  //   x += 160;
  // }
  // return x;
}
