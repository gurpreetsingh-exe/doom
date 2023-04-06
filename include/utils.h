#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#define FORCE_INLINE __attribute__((always_inline)) inline

/// map X from A - B to C - D
#define MAP_RANGE(X, A, B, C, D) (X - A) / (B - A) * (D - C) + C
#define MAX(a, b) ((a > b) ? a : b)
#define MIN(a, b) ((a < b) ? a : b)

uint32_t convert_to_rgba(float x, float y, float z, float w);

#endif // !UTILS_H
