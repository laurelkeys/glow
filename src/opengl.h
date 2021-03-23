#pragma once

#include "prelude.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#define INFO_LOG_LENGTH 512

typedef struct WindowSettings {
    int width;
    int height;
    void (*set_callbacks_fn)(GLFWwindow *window);
    bool vsync;
    bool fullscreen; // (windowed mode)
} WindowSettings;

GLFWwindow *init_opengl(WindowSettings settings, Err *err);
bool shader_compile_success(uint shader, char info_log[INFO_LOG_LENGTH], Err *err);
bool program_link_success(uint program, char info_log[INFO_LOG_LENGTH], Err *err);
