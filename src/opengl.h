#pragma once

#include "prelude.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#define INFO_LOG_LENGTH 512

// clang-format off
static char const *vertex_shader_source =
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main() {\n"
    "    gl_Position = vec4(aPos, 1.0);\n"
    "}\0";
static char const *fragment_shader_source =
    "#version 330 core\n"
    "out vec4 fragColor;\n"
    "void main() {\n"
    "    fragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\0";
// clang-format on

GLFWwindow *init_opengl(int render_width, int render_height, Err *err);
bool shader_compile_success(uint shader, char info_log[INFO_LOG_LENGTH]);
bool program_link_success(uint program, char info_log[INFO_LOG_LENGTH]);
