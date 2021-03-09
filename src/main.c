#include "prelude.h"

#include "console.h"
#include "maths.h"
#include "opengl.h"

#include <stdio.h>

#define RENDER_WIREFRAME false

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
            GLOW_WARNING("vertex shader compilation failed with ```\n%s```", info_log);
        }

        uint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
        glCompileShader(fragment_shader);
        if (!shader_compile_success(fragment_shader, info_log)) {
            GLOW_WARNING("fragment shader compilation failed with ```\n%s```", info_log);
        }

        glAttachShader(shader_program, vertex_shader);
        glAttachShader(shader_program, fragment_shader);
        glLinkProgram(shader_program);
        if (!program_link_success(fragment_shader, info_log)) {
            GLOW_WARNING("shader program linking failed with ```\n%s```", info_log);
        }

        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
    }

    // clang-format off
    float vertices[] = {
         0.5f,  0.5f, 0.0f, // top right
         0.5f, -0.5f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f, // bottom left
        -0.5f,  0.5f, 0.0f, // top left
    };
    // clang-format on

    uint indices[] = {
        0, 1, 2, // bottom-right triangle
        0, 3, 2, // top-left triangle
    };

    uint vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void *) 0);
        glEnableVertexAttribArray(0);
    }
    glBindVertexArray(0);

#if RENDER_WIREFRAME
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#endif

    while (!glfwWindowShouldClose(window)) {
        process_input(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader_program);
        glBindVertexArray(vao);
        {
            // glDrawArrays(GL_TRIANGLES, 0, 3);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
        /* glBindVertexArray(0); */

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(shader_program);

    goto main_exit;

main_err:
    switch (err) {
        case Err_Glfw_Init: GLOW_ERROR("failed to initialize glfw."); break;
        case Err_Glfw_Window: GLOW_ERROR("failed to create glfw window."); break;
        case Err_Glad_Init: GLOW_ERROR("failed to initialize glad."); break;
        default: assert(false);
    }

main_exit:
    glfwTerminate();
    return err ? EXIT_FAILURE : EXIT_SUCCESS;
}
