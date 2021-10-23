#pragma once

#include "prelude.h"

#define INFO_LOG_LENGTH 1024

// Forward declaration.
typedef struct GLFWwindow GLFWwindow;

typedef struct WindowSettings {
    int width;
    int height;
    void (*set_callbacks_fn)(GLFWwindow *window);
    int msaa; // multisample anti-aliasing
    bool vsync;
    bool fullscreen; // (windowed mode)
} WindowSettings;

GLFWwindow *init_opengl(WindowSettings const settings, Err *err);
void deinit_opengl(GLFWwindow *window);

bool check_bound_framebuffer_is_complete();

bool is_shader_compile_success(uint shader, char info_log[INFO_LOG_LENGTH], Err *err);
bool is_program_link_success(uint program, char info_log[INFO_LOG_LENGTH], Err *err);

int get_uniform_location(uint program, char const *name);
int get_attribute_location(uint program, char const *name);
