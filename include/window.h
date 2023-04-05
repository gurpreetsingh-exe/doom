#ifndef WINDOW_H
#define WINDOW_H

#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  GLFWwindow* handle;
  uint32_t width, height;
} Window;

Window* window_init(uint32_t, uint32_t);
bool window_is_running(Window*);
void window_swap_buffers(Window*);
void window_get_size(Window*);
// double window_get_time() { return glfwGetTime(); }
void window_destroy(Window*);

#endif // !WINDOW_H
