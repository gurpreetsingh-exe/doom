#include "window.h"

int main() {
  Window* window = window_init(600, 600);
  while (window_is_running(window)) {
    window_swap_buffers(window);
  }
  window_destroy(window);
}
