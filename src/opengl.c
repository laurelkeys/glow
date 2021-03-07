#include "prelude.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

static void
framebuffer_size_callback(GLFWwindow *window, int render_width, int render_height) {
    glViewport(0, 0, render_width, render_height);
}

GLFWwindow *init_opengl(int render_width, int render_height, Err *err) {
    glfwInit();

    // https://www.glfw.org/docs/latest/window.html#window_hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window =
        glfwCreateWindow(render_width, render_height, "glow", NULL, NULL);

    if (!window) { return (*err = Err_Glfw, NULL); }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        return (*err = Err_Glad, NULL);
    }

    glViewport(0, 0, render_width, render_height);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    return window;
}
