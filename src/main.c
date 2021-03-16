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

// Global variables.
vec3 camera_position = { 0, 0, 3 };
vec3 camera_forward = { 0, 0, -1 };
vec2 mouse_last = { 400, 300 };

bool first_mouse_event = true;

f32 delta_time = 0;
f32 fovy = 45;
f32 pitch = 0;
f32 yaw = -90;

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

    f32 last_frame = 0;
    f32 const aspect_ratio = (f32) window_settings.width / (f32) window_settings.height;
    f32 const near = 0.1f;
    f32 const far = 100.0f;

    while (!glfwWindowShouldClose(window)) {
        f32 curr_frame = glfwGetTime();
        delta_time = curr_frame - last_frame;
        last_frame = curr_frame;

        process_input(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        bind_texture_to_unit(texture1, GL_TEXTURE0);
        bind_texture_to_unit(texture2, GL_TEXTURE1);

        vec3 const target = vec3_add(camera_position, camera_forward);

        mat4 projection = mat4_perspective(RADIANS(fovy), aspect_ratio, near, far);
        mat4 const view = mat4_lookat(camera_position, target, vec3_unit_y());

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
    if PRESSED (ESCAPE) { glfwSetWindowShouldClose(window, true); }

    f32 const camera_speed = 2.5f * delta_time;
    vec3 const forward_backward = vec3_scl(camera_forward, camera_speed);
    vec3 const right_left =
        vec3_scl(vec3_normalize(vec3_cross(camera_forward, vec3_unit_y())), camera_speed);

    if PRESSED (W) { camera_position = vec3_add(camera_position, forward_backward); }
    if PRESSED (S) { camera_position = vec3_sub(camera_position, forward_backward); }
    if PRESSED (A) { camera_position = vec3_sub(camera_position, right_left); }
    if PRESSED (D) { camera_position = vec3_add(camera_position, right_left); }
}

#undef PRESSED

//
// Window callbacks.
//

static void framebuffer_size_callback(GLFWwindow *window, int render_width, int render_height) {
    glViewport(0, 0, render_width, render_height);
}
static void cursor_pos_callback(GLFWwindow *window, f64 xpos, f64 ypos) {
    if (first_mouse_event) {
        mouse_last.x = xpos;
        mouse_last.y = ypos;
        first_mouse_event = false;
    }

    f32 const xoffset = xpos - mouse_last.x;
    f32 const yoffset = mouse_last.y - ypos;
    mouse_last.x = xpos;
    mouse_last.y = ypos;

    pitch += yoffset * 0.1f;
    pitch = CLAMP(pitch, -89.0f, 89.0f);
    yaw += xoffset * 0.1f;

    camera_forward = vec3_normalize((vec3) {
        .x = cosf(RADIANS(yaw)) * cosf(RADIANS(pitch)),
        .y = sinf(RADIANS(pitch)),
        .z = sinf(RADIANS(yaw)) * cosf(RADIANS(pitch)),
    });
}
static void scroll_callback(GLFWwindow *window, f64 xoffset, f64 yoffset) {
    fovy -= (f32) yoffset;
    fovy = CLAMP(fovy, 1.0f, 45.0f);
}

void set_window_callbacks(GLFWwindow *window) {
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetScrollCallback(window, scroll_callback);
}