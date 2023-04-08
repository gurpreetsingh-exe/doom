#ifndef VEC2_H
#define VEC2_H

#include "utils.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include "window.h"
extern Window* window;

#define VEC2_ZERO                                                              \
  (Vec2) { .x = 0, .y = 0 }

typedef struct {
  int16_t x, y;
} Vec2;

FORCE_INLINE Vec2 vec2(int16_t x, int16_t y) { return (Vec2){.x = x, .y = y}; }

FORCE_INLINE Vec2 vec2_add(Vec2 v1, Vec2 v2) {
  return (Vec2){.x = v1.x + v2.x, .y = v1.y + v2.y};
}

FORCE_INLINE Vec2 vec2_sub(Vec2 v1, Vec2 v2) {
  return (Vec2){.x = v1.x - v2.x, .y = v1.y - v2.y};
}

FORCE_INLINE Vec2 vec2_max(Vec2 v1, Vec2 v2) {
  return (Vec2){.x = MAX(v1.x, v2.x), .y = MAX(v1.y, v2.y)};
}

FORCE_INLINE Vec2 vec2_min(Vec2 v1, Vec2 v2) {
  return (Vec2){.x = MIN(v1.x, v2.x), .y = MIN(v1.y, v2.y)};
}

FORCE_INLINE Vec2 vec2_remap(Vec2 v, Vec2 from_min, Vec2 from_max, Vec2 to_min,
                             Vec2 to_max) {
  return vec2(
      MAP_RANGE((float)v.x, from_min.x, from_max.x, to_min.x, to_max.x),
      MAP_RANGE((float)v.y, from_min.y, from_max.y, to_min.y, to_max.y));
}

FORCE_INLINE Vec2 vec2_remap_window(Vec2 v, Vec2 min_pos, Vec2 max_pos) {
  return vec2_remap(v, min_pos, max_pos, VEC2_ZERO,
                    vec2(window->width / 2 - 1, window->height / 2 - 1));
}

FORCE_INLINE Vec2 rotate(Vec2 v, float angle) {
  return vec2(v.x * cos(angle) - v.y * sin(angle),
              v.x * sin(angle) + v.y * cos(angle));
}

FORCE_INLINE void print_vec2(Vec2 v) {
  printf("vec2(x: %d, y: %d)\n", v.x, v.y);
}

#endif // !VEC2_H
