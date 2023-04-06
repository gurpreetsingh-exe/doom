#ifndef WINDOW_H
#define WINDOW_H

#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  GLFWwindow* handle;
  uint32_t width, height;
} Window;

Window* window_init(uint32_t width, uint32_t height);
bool window_is_running(Window*);
void window_get_size(Window*);
void window_destroy(Window*);

#endif // !WINDOW_H
