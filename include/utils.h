#ifndef UTILS_H
#define UTILS_H

/// map X from A - B to C - D
#define MAP_RANGE(X, A, B, C, D) (X - A) / (B - A) * (D - C) + C
#define MAX(a, b) ((a > b) ? a : b)
#define MIN(a, b) ((a < b) ? a : b)

#endif // !UTILS_H
