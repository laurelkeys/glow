#include "opengl.h"

#include "console.h"
#include "maths.h"

static void error_callback(int error, char const *description) {
    GLOW_WARNING("glfw error %d: %s", error, description);
}

static void framebuffer_size_callback(GLFWwindow *window, int render_width, int render_height) {
    glViewport(0, 0, render_width, render_height);
}

static void set_window_callbacks(GLFWwindow *window) {
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // ...
}

GLFWwindow *init_opengl(WindowSettings settings, Err *err) {
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) { return (*err = Err_Glfw_Init, NULL); }

    // https://www.glfw.org/docs/latest/window.html#window_hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow *window = NULL;
    if (settings.fullscreen) {
        GLFWmonitor *monitor = glfwGetPrimaryMonitor();
        GLFWvidmode const *mode = glfwGetVideoMode(monitor);
        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
        window = glfwCreateWindow(mode->width, mode->height, "glow", monitor, NULL);
    } else {
        window = glfwCreateWindow(settings.width, settings.height, "glow", NULL, NULL);
    }

    if (!window) { return (*err = Err_Glfw_Window, NULL); }

    glfwMakeContextCurrent(window);
    set_window_callbacks(window);
    if (settings.vsync) { glfwSwapInterval(1); }

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        return (*err = Err_Glad_Init, NULL);
    }

    return window;
}

bool shader_compile_success(uint shader, char info_log[INFO_LOG_LENGTH], Err *err) {
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        int length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        glGetShaderInfoLog(shader, INFO_LOG_LENGTH, NULL, info_log);
        info_log[CLAMP(length - 1, 0, INFO_LOG_LENGTH - 1)] = '\0';
        return (*err = Err_Shader_Compile, false);
    }
    return true;
}

bool program_link_success(uint program, char info_log[INFO_LOG_LENGTH], Err *err) {
    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        int length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        glGetProgramInfoLog(program, INFO_LOG_LENGTH, NULL, info_log);
        info_log[CLAMP(length - 1, 0, INFO_LOG_LENGTH - 1)] = '\0';
        return (*err = Err_Shader_Link, false);
    }
    return true;
}
