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
    if (err) { goto main_err; }

    uint shader_program = glCreateProgram();
    {
        char info_log[INFO_LOG_LENGTH];

        uint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
        glCompileShader(vertex_shader);
        if (!shader_compile_success(vertex_shader, info_log)) {
            fprintf(
                stderr, "[glow] Warning: vertex shader failed. ```\n%s```\n", info_log);
        }

        uint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
        glCompileShader(fragment_shader);
        if (!shader_compile_success(fragment_shader, info_log)) {
            fprintf(
                stderr, "[glow] Warning: fragment shader failed. ```\n%s```\n", info_log);
        }

        glAttachShader(shader_program, vertex_shader);
        glAttachShader(shader_program, fragment_shader);
        glLinkProgram(shader_program);
        if (!program_link_success(fragment_shader, info_log)) {
            fprintf(
                stderr, "[glow] Warning: program link failed. ```\n%s```\n", info_log);
        }

        glDeleteShader(fragment_shader);
        glDeleteShader(vertex_shader);
    }

    // clang-format off
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
    };
    // clang-format on

    uint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void *) 0);
    glEnableVertexAttribArray(0);

    while (!glfwWindowShouldClose(window)) {
        process_input(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader_program);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    goto main_exit;

main_err:
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

main_exit:
    glfwTerminate();
    return err ? EXIT_FAILURE : EXIT_SUCCESS;
}
