#include "prelude.h"

#include "maths.h"
#include "opengl.h"

#include <stdio.h>

void process_input(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

int main(int argc, char *argv[]) {
    Err err = Err_None;

    GLFWwindow *const window = init_opengl(800, 600, &err);
    if (err) { goto glow_err_main; }

    while (!glfwWindowShouldClose(window)) {
        process_input(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return EXIT_SUCCESS;

glow_err_main:
    switch (err) {
        case Err_Glfw:
            fprintf(stderr, "[glow] Error: failed to create glfw window.\n");
            glfwTerminate();
            break;
        case Err_Glad:
            fprintf(stderr, "[glow] Error: failed to initialize glad.\n");
            break;
        // case Err_Unkown: break;
        default: assert(false);
    }
    return EXIT_FAILURE;
}
