#include "prelude.h"

#include "console.h"
#include "maths.h"
#include "opengl.h"
#include "shader.h"
#include "texture.h"

#include <stdio.h>

#include <stb_image.h>

#define SHADERS_ "src/shaders/"
#define TEXTURES_ "res/textures/"

void process_input(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

int main(int argc, char *argv[]) {
    Err err = Err_None;

    GLFWwindow *const window = init_opengl((WindowSettings) { 800, 600 }, &err);
    if (err) { goto main_err; }

    Shader shader = new_shader_from_filepath(SHADERS_ "default.vs", SHADERS_ "default.fs", &err);
    if (err) { goto main_err; }

    // clang-format off
    f32 const vertices[] = {
        // positions         // colors          // texture coords
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,  // bottom left
         0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,  // bottom right
         0.5f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,  // top right
        -0.5f,  0.5f, 0.0f,  1.0f, 1.0f, 0.0f,  0.0f, 1.0f,  // top left
    };

    uint const indices[] = {
        0, 1, 2, // bottom-right triangle
        0, 2, 3, // top-left triangle
    };
    // clang-format on

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

        uint const stride = sizeof(f32) * 8;
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void *) 0);
        glEnableVertexAttribArray(0); // position attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void *) (sizeof(f32) * 3));
        glEnableVertexAttribArray(1); // color attribute
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void *) (sizeof(f32) * 6));
        glEnableVertexAttribArray(2); // texture coord attribute
    }
    glBindVertexArray(0);

    stbi_set_flip_vertically_on_load(true);
    Texture const texture1 = new_texture_from_filepath(TEXTURES_ "container.jpg", &err);
    if (err) { goto main_err; }
    Texture const texture2 = new_texture_from_filepath(TEXTURES_ "awesomeface.png", &err);
    if (err) { goto main_err; }
    use_shader(shader);
    set_shader_int(shader, "texture1", 0);
    set_shader_int(shader, "texture2", 1);

    while (!glfwWindowShouldClose(window)) {
        process_input(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        bind_texture_to_unit(texture1, GL_TEXTURE0);
        bind_texture_to_unit(texture2, GL_TEXTURE1);

        use_shader(shader);
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(shader.program_id);

    goto main_exit;

main_err:
    switch (err) {
        case Err_Glfw_Init: GLOW_ERROR("failed to initialize glfw"); break;
        case Err_Glfw_Window: GLOW_ERROR("failed to create glfw window"); break;
        case Err_Glad_Init: GLOW_ERROR("failed to initialize glad"); break;
        case Err_Shader_Compile: GLOW_ERROR("failed to compile shader"); break;
        case Err_Shader_Link: GLOW_ERROR("failed to link shader program"); break;
        case Err_Stbi_Load: GLOW_ERROR("stbi_load() failed"); break;
        case Err_Fopen: GLOW_ERROR("fopen() failed"); break;
        case Err_Malloc: GLOW_ERROR("malloc() failed"); break;
        default: assert(false);
    }

main_exit:
    glfwTerminate();
    return err ? EXIT_FAILURE : EXIT_SUCCESS;
}
