#include "prelude.h"

#include "camera.h"
#include "console.h"
#include "maths.h"
#include "opengl.h"
#include "shader.h"
#include "texture.h"

#include <stdio.h>

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

// Global variables.
Camera camera;

vec2 mouse_last;
bool mouse_is_first = true;
bool is_tab_pressed = false;

Shader cube_shader;
Shader light_cube_shader;

// Forward declarations.
void process_input(GLFWwindow *window, f32 const delta_time);
void set_window_callbacks(GLFWwindow *window);

int main(int argc, char *argv[]) {
    Err err = Err_None;

    printf("GLOW_SHADERS_ = " GLOW_SHADERS_ "\n");
    printf("GLOW_TEXTURES_ = " GLOW_TEXTURES_ "\n");

    return 0;

    WindowSettings const window_settings = { 800, 600, set_window_callbacks };
    mouse_last.x = (f32) window_settings.width / 2.0f;
    mouse_last.y = (f32) window_settings.height / 2.0f;

    camera = new_camera_at((vec3) { 0, 0, 3 });
    camera.aspect = (f32) window_settings.width / (f32) window_settings.height;

    GLFWwindow *const window = init_opengl(window_settings, &err);
    if (err) { goto main_err; }

    // @Volatile: use these same files in `process_input`.
    cube_shader = TRY_NEW_SHADER("cube", &err);
    light_cube_shader = TRY_NEW_SHADER("light_cube", &err);

    Texture const diffuse_map = TRY_NEW_TEXTURE("container2.png", &err);
    Texture const specular_map = TRY_NEW_TEXTURE("container2_specular.png", &err);

    // clang-format off
    f32 const cube_vertices[] = {
        // positions            // normals             // texture coords
        -0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,    0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,    1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,    1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,    1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,    0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,    0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,     0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,     1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,     1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,     1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,     0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,     0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,    1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,    1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,    0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,    0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,    0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,    1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,    1.0f,  0.0f,  0.0f,    1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,    1.0f,  0.0f,  0.0f,    1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,    1.0f,  0.0f,  0.0f,    0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,    1.0f,  0.0f,  0.0f,    0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,    1.0f,  0.0f,  0.0f,    0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,    1.0f,  0.0f,  0.0f,    1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,    0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,    1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,    1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,    1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,    0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,    0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,    0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,    1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,    1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,    1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,    0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,    0.0f, 1.0f,
    };
    // clang-format on

    uint vao_cube, vao_light_cube, vbo;
    glGenVertexArrays(1, &vao_cube);
    glGenVertexArrays(1, &vao_light_cube);
    glGenBuffers(1, &vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);
    // @Note: the VBO now contains the cube data we will need for both VAOs,
    // and since it's now bound (and we do not bind any other VBOs) we don't
    // need to rebind it after the VAOs to link it with glVertexAttribPointer.

    uint const stride = sizeof(f32) * 8;

    glBindVertexArray(vao_cube);
    {
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void *) 0);
        glEnableVertexAttribArray(0); // position attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void *) (sizeof(f32) * 3));
        glEnableVertexAttribArray(1); // normal attribute
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void *) (sizeof(f32) * 6));
        glEnableVertexAttribArray(2); // texcoord attribute
    }
    glBindVertexArray(vao_light_cube);
    {
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void *) 0);
        glEnableVertexAttribArray(0); // position attribute
    }
    glBindVertexArray(0);

    f32 const near = 0.1f;
    f32 const far = 100.0f;

    f32 last_frame = 0; // time of last frame
    f32 delta_time = 0; // time between consecutive frames

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // @Cleanup

    while (!glfwWindowShouldClose(window)) {
        f32 const curr_frame = glfwGetTime();
        delta_time = curr_frame - last_frame;
        last_frame = curr_frame;
        process_input(window, delta_time);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mat4 const projection = mat4_perspective(RADIANS(camera.fovy), camera.aspect, near, far);
        mat4 const view = get_camera_view_matrix(&camera);

        vec3 const light_pos = { 1.2f, 1.0f, 2.0f };

        use_shader(cube_shader);
        {
            set_shader_mat4(cube_shader, "local_to_world", mat4_id());
            set_shader_mat4(cube_shader, "world_to_view", view);
            set_shader_mat4(cube_shader, "view_to_clip", projection);

            set_shader_vec3(cube_shader, "light_in_world", light_pos);

            set_shader_vec3(cube_shader, "light.ambient", vec3_of(0.2f));
            set_shader_vec3(cube_shader, "light.diffuse", vec3_of(1.0f));
            set_shader_vec3(cube_shader, "light.specular", vec3_of(1.0f));

            set_shader_int(cube_shader, "material.diffuse", 0);
            set_shader_int(cube_shader, "material.specular", 1);
            set_shader_float(cube_shader, "material.shininess", 32.0f);

            bind_texture_to_unit(diffuse_map, GL_TEXTURE0);
            bind_texture_to_unit(specular_map, GL_TEXTURE1);

            glBindVertexArray(vao_cube);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        use_shader(light_cube_shader);
        {
            mat4 const model = mat4_mul(mat4_translate(light_pos), mat4_scale(vec3_of(0.2f)));
            set_shader_mat4(light_cube_shader, "local_to_world", model);
            set_shader_mat4(light_cube_shader, "world_to_view", view);
            set_shader_mat4(light_cube_shader, "view_to_clip", projection);

            glBindVertexArray(vao_light_cube);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao_light_cube);
    glDeleteVertexArrays(1, &vao_cube);
    glDeleteProgram(light_cube_shader.program_id);
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
        case Err_Fopen: GLOW_ERROR("fopen() failed"); break;
        case Err_Malloc: GLOW_ERROR("malloc() failed"); break;
        case Err_Calloc: GLOW_ERROR("calloc() failed"); break;
        default: assert(false);
    }

main_exit:
    glfwTerminate();
    return err ? EXIT_FAILURE : EXIT_SUCCESS;
}

//
// Input processing.
//

#define IS_PRESSED(key) (glfwGetKey(window, GLFW_KEY_##key) == GLFW_PRESS)

#define ON_PRESS(key, is_key_pressed)                                              \
    (glfwGetKey(window, GLFW_KEY_##key) != GLFW_PRESS) { is_key_pressed = false; } \
    else if (!is_key_pressed && (is_key_pressed = true))

void process_input(GLFWwindow *window, f32 const delta_time) {
    if IS_PRESSED (ESCAPE) { glfwSetWindowShouldClose(window, true); }

    if IS_PRESSED (W) { update_camera_position(&camera, CameraMovement_Forward, delta_time); }
    if IS_PRESSED (S) { update_camera_position(&camera, CameraMovement_Backward, delta_time); }
    if IS_PRESSED (A) { update_camera_position(&camera, CameraMovement_Left, delta_time); }
    if IS_PRESSED (D) { update_camera_position(&camera, CameraMovement_Right, delta_time); }
    if IS_PRESSED (E) { update_camera_position(&camera, CameraMovement_Up, delta_time); }
    if IS_PRESSED (Q) { update_camera_position(&camera, CameraMovement_Down, delta_time); }

    if ON_PRESS (TAB, is_tab_pressed) {
        // @Volatile: use the same files as in `main`.

        GLOW_LOG("Hot swapping 'cube' shaders");
        reload_shader_from_filepath(
            &cube_shader, GLOW_SHADERS_ "cube.vs", GLOW_SHADERS_ "cube.fs");

        GLOW_LOG("Hot swapping 'light_cube' shaders");
        reload_shader_from_filepath(
            &light_cube_shader, GLOW_SHADERS_ "light_cube.vs", GLOW_SHADERS_ "light_cube.fs");
    }
}

#undef ON_PRESS
#undef IS_PRESSED

//
// Window callbacks.
//

static void framebuffer_size_callback(GLFWwindow *window, int render_width, int render_height) {
    glViewport(0, 0, render_width, render_height);
    camera.aspect = (f32) render_width / (f32) render_height;
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

void set_window_callbacks(GLFWwindow *window) {
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetScrollCallback(window, scroll_callback);
}
