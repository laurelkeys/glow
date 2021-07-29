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

void imgui_show_demo_window(void);

#ifdef __cplusplus
}
#endif
