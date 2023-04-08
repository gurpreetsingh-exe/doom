#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#define FORCE_INLINE __attribute__((always_inline)) inline

/// map X from A - B to C - D
#define MAP_RANGE(X, A, B, C, D) (X - A) / (B - A) * (D - C) + C
#define MAX(a, b) ((a > b) ? a : b)
#define MIN(a, b) ((a < b) ? a : b)
#define PI 3.14159265359
#define RADIANS(a) ((a)*PI / 180)
#define DEGREES(a) ((a)*180 / PI)

uint32_t convert_to_rgba(float x, float y, float z, float w);
float norm_angle(float angle);
// int angle_to_screen(float angle);
int angle_to_screen(float angle, int hw);

#endif // !UTILS_H
