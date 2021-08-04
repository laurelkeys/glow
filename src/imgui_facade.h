#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration.
typedef struct GLFWwindow GLFWwindow;

void init_imgui(GLFWwindow *window);
void deinit_imgui(void);

void begin_imgui_frame(void);
void end_imgui_frame(void);

void show_imgui_demo_window(void);

void imgui_config_mouse(bool should_capture);

#ifdef __cplusplus
}
#endif
