#include "prelude.h"

#include "maths.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <stdio.h>

typedef enum {
    glow_Error_None = 0,
    glow_Error_Glfw,
    glow_Error_Glad,
} glow_Error;

typedef struct {
    glow_Error err;
    GLFWwindow *window;
} init_opengl_Return;

init_opengl_Return init_opengl(int render_width, int render_height);

int main(int argc, char *argv[]) {
    init_opengl_Return ret = init_opengl(800, 600);
    if (ret.err) { return EXIT_FAILURE; }

    GLFWwindow *const window = ret.window;
    while (!glfwWindowShouldClose(window)) {
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return EXIT_SUCCESS;
}

//
// OpenGL initialization.
//

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

init_opengl_Return init_opengl(int render_width, int render_height) {
    glfwInit();

    // https://www.glfw.org/docs/latest/window.html#window_hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window =
        glfwCreateWindow(render_width, render_height, "glow", NULL, NULL);

    if (!window) {
        fprintf(stderr, "[glow] Error: failed to create glfw window.\n");
        glfwTerminate();
        return (init_opengl_Return) { .err = glow_Error_Glfw };
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        fprintf(stderr, "[glow] Error: failed to initialize glad.\n");
        return (init_opengl_Return) { .err = glow_Error_Glad };
    }

    glViewport(0, 0, render_width, render_height);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    return (init_opengl_Return) { .window = window };
}

void framebuffer_size_callback(GLFWwindow *window, int render_width, int render_height) {
    glViewport(0, 0, render_width, render_height);
}
