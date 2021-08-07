#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration.
typedef struct GLFWwindow GLFWwindow;

void init_imgui(GLFWwindow* window);
void deinit_imgui(void);

void begin_imgui_frame(void);
void end_imgui_frame(void);

void show_imgui_demo_window(void);

void imgui_config_mouse(bool should_capture);

void imgui_slider_int(char const* label, int* v, int v_min, int v_max);
void imgui_slider_float(char const* label, float* v, float v_min, float v_max);

#ifdef __cplusplus
}
#endif
