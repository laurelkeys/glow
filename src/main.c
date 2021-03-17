#include "prelude.h"

#include "camera.h"
#include "console.h"
#include "maths.h"
#include "opengl.h"
#include "shader.h"
#include "texture.h"

#include <stdio.h>

#include <stb_image.h>

#define SHADERS_ "src/shaders/"
#define TEXTURES_ "res/textures/"

// Global variables.
Camera camera;
vec2 mouse_last = { 400, 300 };
bool mouse_is_first = true;
f32 delta_time = 0;

// Forward declarations.
void process_input(GLFWwindow *window);
void set_window_callbacks(GLFWwindow *window);

int main(int argc, char *argv[]) {
    Err err = Err_None;

    WindowSettings const window_settings = {
        .width = 800,
        .height = 600,
        .fullscreen = false,
        .vsync = true,
        .set_callbacks_fn = set_window_callbacks,
    };

    GLFWwindow *const window = init_opengl(window_settings, &err);
    if (err) { goto main_err; }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // @Cleanup

    Shader shader = new_shader_from_filepath(SHADERS_ "default.vs", SHADERS_ "default.fs", &err);
    if (err) { goto main_err; }

    // clang-format off
    f32 const vertices[] = {
        // positions          // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    };
    // clang-format on

    uint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        uint const stride = sizeof(f32) * 5;
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void *) 0);
        glEnableVertexAttribArray(0); // position attribute
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void *) (sizeof(f32) * 3));
        glEnableVertexAttribArray(1); // texture coord attribute
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

    // clang-format off
    vec3 const cube_positions[] = {
        {  0.0f,  0.0f,   0.0f },
        {  2.0f,  5.0f, -15.0f },
        { -1.5f, -2.2f,  -2.5f },
        { -3.8f, -2.0f, -12.3f },
        {  2.4f, -0.4f,  -3.5f },
        { -1.7f,  3.0f,  -7.5f },
        {  1.3f, -2.0f,  -2.5f },
        {  1.5f,  2.0f,  -2.5f },
        {  1.5f,  0.2f,  -1.5f },
        { -1.3f,  1.0f,  -1.5f },
    };
    // clang-format on

    f32 const aspect_ratio = (f32) window_settings.width / (f32) window_settings.height;
    f32 const near = 0.1f;
    f32 const far = 100.0f;

    f32 last_frame = 0;
    camera = new_camera_at((vec3) { 0, 0, 3 });

    while (!glfwWindowShouldClose(window)) {
        f32 curr_frame = glfwGetTime();
        delta_time = curr_frame - last_frame;
        last_frame = curr_frame;

        process_input(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        bind_texture_to_unit(texture1, GL_TEXTURE0);
        bind_texture_to_unit(texture2, GL_TEXTURE1);

        mat4 const projection = mat4_perspective(RADIANS(camera.fovy), aspect_ratio, near, far);
        mat4 const view = get_camera_matrix(&camera);

        use_shader(shader);
        set_shader_mat4(shader, "world_to_view", view);
        set_shader_mat4(shader, "view_to_clip", projection);

        glBindVertexArray(vao);
        for (int i = 0; i < ARRAY_LEN(cube_positions); ++i) {
            f32 const angle = RADIANS((i % 3) ? 20 * i : 25 * glfwGetTime());
            mat4 const model = mat4_mul(
                mat4_translate(cube_positions[i]),
                mat4_rotate(angle, (vec3) { 1.0f, 0.3f, 0.5f }));

            set_shader_mat4(shader, "local_to_world", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

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

//
// Input processing.
//

#define PRESSED(key) (glfwGetKey(window, GLFW_KEY_##key) == GLFW_PRESS)

void process_input(GLFWwindow *window) {
    if (PRESSED(ESCAPE)) { glfwSetWindowShouldClose(window, true); }

    if (PRESSED(W)) { update_camera_position(&camera, CameraMovement_Forward, delta_time); }
    if (PRESSED(S)) { update_camera_position(&camera, CameraMovement_Backward, delta_time); }
    if (PRESSED(A)) { update_camera_position(&camera, CameraMovement_Left, delta_time); }
    if (PRESSED(D)) { update_camera_position(&camera, CameraMovement_Right, delta_time); }
    // @Todo: use E and Q for up and down.
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

    // Reverse y since 0, 0 is the top-left.
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
