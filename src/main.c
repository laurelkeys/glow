#include "prelude.h"

#include "camera.h"
#include "console.h"
#include "maths.h"
#include "opengl.h"
#include "shader.h"
#include "texture.h"

#include <stdio.h>

#include <stb_image.h>

// @Cleanup: set paths using CMake's PROJECT_SOURCE_DIR
#define SHADERS_ "src/shaders/"
#define TEXTURES_ "res/textures/"

// Global variables.
Camera camera;

vec2 mouse_last;
bool mouse_is_first = true;

Shader cube_shader;
Shader light_cube_shader;

// Forward declarations.
void process_input(GLFWwindow *window, f32 const delta_time);
void set_window_callbacks(GLFWwindow *window);

int main(int argc, char *argv[]) {
    Err err = Err_None;

    WindowSettings const window_settings = { 800, 600, .set_callbacks_fn = set_window_callbacks };
    mouse_last.x = (f32) window_settings.width / 2.0f;
    mouse_last.y = (f32) window_settings.height / 2.0f;
    camera = new_camera_at((vec3) { 0, 0, 3 });

    GLFWwindow *const window = init_opengl(window_settings, &err);
    if (err) { goto main_err; }

    // @Volatile: use these same files in `key_callback`.
    cube_shader = new_shader_from_filepath(SHADERS_ "cube.vs", SHADERS_ "cube.fs", &err);
    if (err) { goto main_err; }
    light_cube_shader =
        new_shader_from_filepath(SHADERS_ "light_cube.vs", SHADERS_ "light_cube.fs", &err);
    if (err) { goto main_err; }

    vec3 const light_pos = { 1.2f, 1.0f, 2.0f };

    // clang-format off
    f32 const cube_vertices[] = {
        // positions            // normals
        -0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,

         0.5f,  0.5f,  0.5f,    1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,    1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,    1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,    1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,    1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,    1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f
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

    uint const stride = sizeof(f32) * 6;

    glBindVertexArray(vao_cube);
    {
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void *) 0);
        glEnableVertexAttribArray(0); // position attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void *) (sizeof(f32) * 3));
        glEnableVertexAttribArray(1); // normal attribute
    }
    glBindVertexArray(vao_light_cube);
    {
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void *) 0);
        glEnableVertexAttribArray(0); // position attribute
    }
    glBindVertexArray(0);

    // @Fixme: update aspect ratio on the render loop to reflect viewport resizing.
    f32 const aspect_ratio = (f32) window_settings.width / (f32) window_settings.height;
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

        mat4 const projection = mat4_perspective(RADIANS(camera.fovy), aspect_ratio, near, far);
        mat4 const view = get_camera_view_matrix(&camera);

        use_shader(cube_shader);
        {
            set_shader_vec3(cube_shader, "light_pos", light_pos);
            set_shader_vec3(cube_shader, "light_color", (vec3) { 1, 1, 1 });
            set_shader_vec3(cube_shader, "object_color", (vec3) { 1.0f, 0.5f, 0.31f });

            set_shader_mat4(cube_shader, "local_to_world", mat4_id());
            set_shader_mat4(cube_shader, "world_to_view", view);
            set_shader_mat4(cube_shader, "view_to_clip", projection);

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
        default: assert(false);
    }

main_exit:
    glfwTerminate();
    return err ? EXIT_FAILURE : EXIT_SUCCESS;
}

//
// Input processing.
//

#define PRESSED(key) (glfwGetKey(window, GLFW_KEY_##key) == GLFW_PRESS)

void process_input(GLFWwindow *window, f32 const delta_time) {
    if PRESSED (ESCAPE) { glfwSetWindowShouldClose(window, true); }

    if PRESSED (W) { update_camera_position(&camera, CameraMovement_Forward, delta_time); }
    if PRESSED (S) { update_camera_position(&camera, CameraMovement_Backward, delta_time); }
    if PRESSED (A) { update_camera_position(&camera, CameraMovement_Left, delta_time); }
    if PRESSED (D) { update_camera_position(&camera, CameraMovement_Right, delta_time); }
    if PRESSED (E) { update_camera_position(&camera, CameraMovement_Up, delta_time); }
    if PRESSED (Q) { update_camera_position(&camera, CameraMovement_Down, delta_time); }
}

#undef PRESSED

//
// Window callbacks.
//

static void framebuffer_size_callback(GLFWwindow *window, int render_width, int render_height) {
    glViewport(0, 0, render_width, render_height);
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
    if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
        // @Volatile: use the same files as in `main`.
        GLOW_LOG("Hot swapping 'cube' shaders");
        reload_shader_from_filepath(&cube_shader, SHADERS_ "cube.vs", SHADERS_ "cube.fs");
        GLOW_LOG("Hot swapping 'light_cube' shaders");
        reload_shader_from_filepath(
            &light_cube_shader, SHADERS_ "light_cube.vs", SHADERS_ "light_cube.fs");
    }
}

void set_window_callbacks(GLFWwindow *window) {
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
}
