#include "window.h"
#include "GLFW/glfw3.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>

extern Config config;

static void key_callback(GLFWwindow* window, int key, int scancode, int action,
                         int mods) {
  Event* event = (Event*)(glfwGetWindowUserPointer(window));
  if (action == GLFW_PRESS) {
    if (key == GLFW_KEY_TAB) {
      config.top_view = !config.top_view;
    }
    event->pressed[key] = true;
  } else if (action == GLFW_RELEASE) {
    event->pressed[key] = false;
  }
}

Window* window_init(uint32_t width, uint32_t height) {
  if (!glfwInit()) {
    fprintf(stderr, "glfw initialization failed\n");
    exit(1);
  }

  Window* window = (Window*)malloc(sizeof(Window));
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  window->handle = glfwCreateWindow(width, height, "Doom Engine", NULL, NULL);
  window->width = width;
  window->height = height;

  if (!window) {
    fprintf(stderr, "failed to create window\n");
    exit(1);
  }

  glfwMakeContextCurrent(window->handle);
  window->event = (Event*)calloc(1, sizeof(Event));
  glfwSetWindowUserPointer(window->handle, window->event);
  glfwSetKeyCallback(window->handle, key_callback);

  if (glewInit() == GLEW_OK) {
    printf("GL version: %s\n", glGetString(GL_VERSION));
  }

  return window;
}

static double last_time = 0;

bool window_is_running(Window* window) {
  bool is_running = !glfwWindowShouldClose(window->handle);
  glfwPollEvents();
  glfwSwapBuffers(window->handle);
  double time = glfwGetTime();
  window->event->delta_time = (time - last_time) * 1000;
  last_time = time;
  return is_running;
}

void window_get_size(Window* window) {
  glfwGetWindowSize(window->handle, (int*)&window->width,
                    (int*)&window->height);
}

void window_destroy(Window* window) {
  glfwDestroyWindow(window->handle);
  glfwTerminate();
  free(window->event);
  free(window);
}
