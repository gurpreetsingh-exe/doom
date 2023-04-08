#include "imgui_layer.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#define CIMGUI_USE_GLFW
#define CIMGUI_USE_OPENGL3
#include "cimgui_impl.h"

#include "window.h"
extern Window* window;

void imgui_init() {
  igCreateContext(NULL);
  ImGuiIO* io = igGetIO();
  io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
#ifdef IMGUI_HAS_DOCK
  io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
#endif
  ImGui_ImplGlfw_InitForOpenGL(window->handle, true);
  ImGui_ImplOpenGL3_Init("#version 450");
  igStyleColorsDark(NULL);
}

void imgui_begin_frame() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  igNewFrame();
}

void imgui_end_frame() {
  igRender();
  ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
#ifdef IMGUI_HAS_DOCK
  ImGuiIO* io = igGetIO();
  if (io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    GLFWwindow* backup_current_window = glfwGetCurrentContext();
    igUpdatePlatformWindows();
    igRenderPlatformWindowsDefault(NULL, NULL);
    glfwMakeContextCurrent(backup_current_window);
  }
#endif
}

void imgui_destroy() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  igDestroyContext(NULL);
}
