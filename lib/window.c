#include "window.h"
#include "GLFW/glfw3.h"
#include <stdio.h>
#include <stdlib.h>

static void key_callback(GLFWwindow* window, int key, int scancode, int action,
                         int mods) {
  Event* event = (Event*)(glfwGetWindowUserPointer(window));
  if (action == GLFW_PRESS) {
    if (key == GLFW_KEY_ESCAPE) {
      event->disable_cursor = !event->disable_cursor;
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
static double last_time2 = 0;
static int nframe = 0;
#define TIME_DIFF 0.5

bool window_is_running(Window* window) {
  bool is_running = !glfwWindowShouldClose(window->handle);
  glfwPollEvents();
  glfwSwapBuffers(window->handle);
  double time = glfwGetTime();
  double ms = time - last_time;
  window->event->delta_time = (time - last_time2) * 1000;
  nframe++;
  if (ms >= TIME_DIFF) {
    char title[32] = {0};
    snprintf(title, 32, "Doom Engine %3.2f", (float)nframe / ms);
    glfwSetWindowTitle(window->handle, title);
    nframe = 0;
    last_time += TIME_DIFF;
  }
  last_time2 = time;
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
