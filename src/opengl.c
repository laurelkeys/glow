#include "opengl.h"

#include "maths.h"

static void
framebuffer_size_callback(GLFWwindow *window, int render_width, int render_height) {
    glViewport(0, 0, render_width, render_height);
}

GLFWwindow *init_opengl(int render_width, int render_height, Err *err) {
    if (!glfwInit()) { return (*err = Err_Glfw_Init, NULL); }

    // https://www.glfw.org/docs/latest/window.html#window_hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window =
        glfwCreateWindow(render_width, render_height, "glow", NULL, NULL);

    if (!window) { return (*err = Err_Glfw_Window, NULL); }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        return (*err = Err_Glad_Init, NULL);
    }

    return window;
}

bool shader_compile_success(uint shader, char info_log[INFO_LOG_LENGTH]) {
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        int length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        glGetShaderInfoLog(shader, INFO_LOG_LENGTH, NULL, info_log);
        info_log[CLAMP(length - 1, 0, INFO_LOG_LENGTH - 1)] = '\0';
        return false;
    }
    return true;
}

bool program_link_success(uint program, char info_log[INFO_LOG_LENGTH]) {
    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        int length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        glGetProgramInfoLog(program, INFO_LOG_LENGTH, NULL, info_log);
        info_log[CLAMP(length - 1, 0, INFO_LOG_LENGTH - 1)] = '\0';
        return false;
    }
    return true;
}
