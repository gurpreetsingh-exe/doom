// clang-format off
#include <GL/glew.h>
// clang-format on
#include "window.h"
#include "GLFW/glfw3.h"
#include <stdio.h>
#include <stdlib.h>

Window* window_init(uint32_t width, uint32_t height) {
  if (!glfwInit()) {
    fprintf(stderr, "glfw initialization failed\n");
    exit(1);
  }

  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  // glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
  GLFWwindow* window =
      glfwCreateWindow(width, height, "Doom Engine", NULL, NULL);
  if (!window) {
    fprintf(stderr, "failed to create window\n");
    exit(1);
  }

  glfwMakeContextCurrent(window);

  if (glewInit() == GLEW_OK) {
    printf("GL version: %s\n", glGetString(GL_VERSION));
  }

  Window* main_window = (Window*)malloc(sizeof(Window));
  main_window->width = width;
  main_window->height = height;
  main_window->handle = window;
  return main_window;
}

bool window_is_running(Window* window) {
  return !glfwWindowShouldClose(window->handle);
}

void window_get_size(Window* window) {
  glfwGetWindowSize(window->handle, (int*)&window->width,
                    (int*)&window->height);
}

void window_swap_buffers(Window* window) {
  glfwPollEvents();
  glfwSwapBuffers(window->handle);
}

void window_destroy(Window* window) {
  glfwDestroyWindow(window->handle);
  glfwTerminate();
  free(window);
}
