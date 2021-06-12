#include "prelude.h"

#include "camera.h"
#include "console.h"
#include "maths.h"
#include "model.h"
#include "opengl.h"
#include "shader.h"
#include "texture.h"
#include "vertices.h"
#include "window.h"

#include <stdio.h>

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <stb_image.h>

#ifndef GLOW_SHADERS_
#define GLOW_SHADERS_ ""
#endif
#define TRY_NEW_SHADER(name, err_ptr)                                                      \
    new_shader_from_filepath(GLOW_SHADERS_ name ".vs", GLOW_SHADERS_ name ".fs", err_ptr); \
    if (*(err_ptr)) { goto main_err; }

#ifndef GLOW_TEXTURES_
#define GLOW_TEXTURES_ ""
#endif
#define TRY_NEW_TEXTURE(filename, err_ptr)                       \
    new_texture_from_filepath(GLOW_TEXTURES_ filename, err_ptr); \
    if (*(err_ptr)) { goto main_err; }

#ifndef GLOW_MODELS_
#define GLOW_MODELS_ ""
#endif
#define TRY_ALLOC_NEW_MODEL(filename, err_ptr)                     \
    alloc_new_model_from_filepath(GLOW_MODELS_ filename, err_ptr); \
    if (*(err_ptr)) { goto main_err; }

// Global variables.
Camera camera;

vec2 mouse_last;
bool mouse_is_first = true;
bool is_tab_pressed = false;

Shader cube_shader;
Shader skybox_shader;

// Forward declarations.
void setup_shaders(void);
void process_input(GLFWwindow *window, f32 delta_time);
void set_window_callbacks(GLFWwindow *window);

// @Todo: continue from "Geometry Shader" https://learnopengl.com/Advanced-OpenGL/Geometry-Shader

int main(int argc, char *argv[]) {
    Err err = Err_None;

    WindowSettings const window_settings = { 800, 600, set_window_callbacks };
    mouse_last.x = (f32) window_settings.width / 2.0f;
    mouse_last.y = (f32) window_settings.height / 2.0f;

    camera = new_camera_at((vec3) { 0, 0, 3 });
    camera.aspect = (f32) window_settings.width / (f32) window_settings.height;

    GLFWwindow *const window = init_opengl(window_settings, &err);
    if (err) { goto main_err; }

    /* glfwSetWindowUserPointer(GLFWwindow *window, void *pointer); */

    // @Volatile: use these same files in `process_input`.
    cube_shader = TRY_NEW_SHADER("simple_envmap", &err);
    skybox_shader = TRY_NEW_SHADER("simple_skybox", &err);

    stbi_set_flip_vertically_on_load(true);
    Texture const cube = TRY_NEW_TEXTURE("container.jpg", &err);

    stbi_set_flip_vertically_on_load(false);
    Texture const skybox = new_cubemap_texture_from_filepaths(
        (char const *[6]) {
            GLOW_TEXTURES_ "skybox/right.jpg", // +X
            GLOW_TEXTURES_ "skybox/left.jpg", // -X
            GLOW_TEXTURES_ "skybox/top.jpg", // +Y
            GLOW_TEXTURES_ "skybox/bottom.jpg", // -Y
            GLOW_TEXTURES_ "skybox/front.jpg", // +Z
            GLOW_TEXTURES_ "skybox/back.jpg", // -Z
        },
        &err);
    if (err) { goto main_err; }

    uint vao_cube;
    glGenVertexArrays(1, &vao_cube);
    {
        uint vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(CUBE_VERTICES), CUBE_VERTICES, GL_STATIC_DRAW);
        glBindVertexArray(vao_cube);
        DEFER(glBindVertexArray(0)) {
            int const stride = sizeof(f32) * 8;
            glEnableVertexAttribArray(0); // position
            glEnableVertexAttribArray(1); // normal
            glEnableVertexAttribArray(2); // texture coords
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void *) 0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void *) (sizeof(f32) * 3));
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void *) (sizeof(f32) * 6));
        }
        glDeleteBuffers(1, &vbo); // @Note: deleting the VBO is only valid after unbinding the VAO
    }

    uint vao_skybox;
    glGenVertexArrays(1, &vao_skybox);
    {
        uint vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(SKYBOX_VERTICES), SKYBOX_VERTICES, GL_STATIC_DRAW);
        glBindVertexArray(vao_skybox);
        DEFER(glBindVertexArray(0)) {
            glEnableVertexAttribArray(0); // position
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(f32) * 3, (void *) 0);
        }
        glDeleteBuffers(1, &vbo);
    }

    Clock clock = { 0 };
    Fps fps = { 0 };
    setup_shaders();

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // @Cleanup
    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window)) {
        clock_tick(&clock, glfwGetTime());
        update_frame_counter(&fps, clock.time);
        process_input(window, clock.time_increment);

        if (fps.last_update_time == clock.time) {
            char title[32];
            snprintf(title, sizeof(title), "glow | %d fps", fps.rate);
            glfwSetWindowTitle(window, title);
        }

        mat4 const projection = get_camera_projection_matrix(&camera);
        mat4 const view = get_camera_view_matrix(&camera);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        use_shader(cube_shader);
        {
            set_shader_vec3(cube_shader, "camera_pos", camera.position);

            set_shader_mat4(cube_shader, "local_to_world", mat4_id());
            set_shader_mat4(cube_shader, "world_to_view", view);
            set_shader_mat4(cube_shader, "view_to_clip", projection);

            glBindVertexArray(vao_cube);
            bind_texture_to_unit(cube, GL_TEXTURE0);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // Change the depth function to make sure the skybox passes the depth tests.
        glDepthFunc(GL_LEQUAL);
        use_shader(skybox_shader);
        {
            mat4 view_without_translation = view;
            view_without_translation.m[0][3] = 0.0f;
            view_without_translation.m[1][3] = 0.0f;
            view_without_translation.m[2][3] = 0.0f;
            set_shader_mat4(skybox_shader, "world_to_view", view_without_translation);
            set_shader_mat4(skybox_shader, "view_to_clip", projection);

            glBindVertexArray(vao_skybox);
            bind_texture_to_unit(skybox, GL_TEXTURE0);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glDepthFunc(GL_LESS);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao_skybox);
    glDeleteVertexArrays(1, &vao_cube);
    glDeleteProgram(skybox_shader.program_id);
    glDeleteProgram(cube_shader.program_id);

    goto main_exit;

main_err:
    switch (err) {
        case Err_Glfw_Init: GLOW_ERROR("failed to initialize glfw"); break;
        case Err_Glfw_Window: GLOW_ERROR("failed to create glfw window"); break;
        case Err_Glad_Init: GLOW_ERROR("failed to initialize glad"); break;
        case Err_Shader_Compile: GLOW_ERROR("failed to compile shader"); break;
        case Err_Shader_Link: GLOW_ERROR("failed to link shader program"); break;
        case Err_Stbi_Load: GLOW_ERROR("stbi_load() failed"); break;
        case Err_Assimp_Import: GLOW_ERROR("aiImportFile() failed"); break;
        case Err_Assimp_Get_Texture: GLOW_ERROR("aiGetMaterialTexture() failed"); break;
        case Err_Fopen: GLOW_ERROR("fopen() failed"); break;
        case Err_Malloc: GLOW_ERROR("malloc() failed"); break;
        case Err_Calloc: GLOW_ERROR("calloc() failed"); break;
        case Err_Realloc: GLOW_ERROR("realloc() failed"); break;
        default: assert(false);
    }

main_exit:
    glfwDestroyWindow(window);
    glfwTerminate();
    return err ? EXIT_FAILURE : EXIT_SUCCESS;
}

void setup_shaders(void) {
    use_shader(cube_shader);
    set_shader_sampler2D(cube_shader, "skybox", GL_TEXTURE0);

    use_shader(skybox_shader);
    set_shader_sampler2D(skybox_shader, "skybox", GL_TEXTURE0);
}

//
// Input processing.
//

#define IS_PRESSED(key) (glfwGetKey(window, GLFW_KEY_##key) == GLFW_PRESS)

#define ON_PRESS(key, is_key_pressed)                                              \
    (glfwGetKey(window, GLFW_KEY_##key) != GLFW_PRESS) { is_key_pressed = false; } \
    else if (!is_key_pressed && (is_key_pressed = true))

#define RELOAD_SHADER(name, shader_ptr)          \
    GLOW_LOG("Hot swapping '" name "' shaders"); \
    reload_shader_from_filepath((shader_ptr), GLOW_SHADERS_ name ".vs", GLOW_SHADERS_ name ".fs");

void process_input(GLFWwindow *window, f32 delta_time) {
    if IS_PRESSED (ESCAPE) { glfwSetWindowShouldClose(window, true); }

    if IS_PRESSED (W) { update_camera_position(&camera, CameraMovement_Forward, delta_time); }
    if IS_PRESSED (S) { update_camera_position(&camera, CameraMovement_Backward, delta_time); }
    if IS_PRESSED (A) { update_camera_position(&camera, CameraMovement_Left, delta_time); }
    if IS_PRESSED (D) { update_camera_position(&camera, CameraMovement_Right, delta_time); }
    if IS_PRESSED (E) { update_camera_position(&camera, CameraMovement_Up, delta_time); }
    if IS_PRESSED (Q) { update_camera_position(&camera, CameraMovement_Down, delta_time); }

    if ON_PRESS (TAB, is_tab_pressed) {
        // @Volatile: use the same files as in `main`.
        RELOAD_SHADER("simple_envmap", &cube_shader);
        RELOAD_SHADER("simple_skybox", &skybox_shader);
        setup_shaders();
    }
}

#undef RELOAD_SHADER
#undef ON_PRESS
#undef IS_PRESSED

//
// Window callbacks.
//

static void framebuffer_size_callback(GLFWwindow *window, int render_width, int render_height) {
    glViewport(0, 0, render_width, render_height);
    camera.aspect = (f32) render_width / (f32) render_height;
}
static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    // Do nothing.
}
static void cursor_pos_callback(GLFWwindow *window, f64 xpos, f64 ypos) {
    if (mouse_is_first) {
        mouse_last.x = xpos;
        mouse_last.y = ypos;
        mouse_is_first = false;
    }

    // Reverse y since 0, 0 is the top left.
    f32 const xoffset = xpos - mouse_last.x;
    f32 const yoffset = mouse_last.y - ypos;
    update_camera_angles(&camera, (CameraMouseEvent) { xoffset, yoffset });

    mouse_last.x = xpos;
    mouse_last.y = ypos;
}
static void scroll_callback(GLFWwindow *window, f64 xoffset, f64 yoffset) {
    update_camera_fovy(&camera, (f32) yoffset);
}
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    // Do nothing.
}

void set_window_callbacks(GLFWwindow *window) {
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
}
