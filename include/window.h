#ifndef WINDOW_H
#define WINDOW_H

// clang-format off
#include <GL/glew.h>
// clang-format on
#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  bool pressed[GLFW_KEY_LAST + 1];
  double delta_time;
} Event;

typedef struct {
  GLFWwindow* handle;
  Event* event;
  uint32_t width, height;
} Window;

Window* window_init(uint32_t width, uint32_t height);
bool window_is_running(Window*);
void window_get_size(Window*);
void window_destroy(Window*);

#endif // !WINDOW_H
