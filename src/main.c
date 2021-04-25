#include "prelude.h"

#include "camera.h"
#include "console.h"
#include "maths.h"
#include "model.h"
#include "opengl.h"
#include "primitives.h"
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

Shader shader;

// Forward declarations.
void process_input(GLFWwindow *window, f32 delta_time);
void set_window_callbacks(GLFWwindow *window);

int transparent_cmp(void const *a, void const *b) {
    f32 const dist_a = vec3_length(vec3_sub(camera.position, *(vec3 const *) a));
    f32 const dist_b = vec3_length(vec3_sub(camera.position, *(vec3 const *) b));
    /* return (dist_a < dist_b) ? 1 : (dist_a > dist_b) ? -1 : 0; */
    return (dist_a < dist_b) - (dist_a > dist_b);
}

int main(int argc, char *argv[]) {
    Err err = Err_None;

    WindowSettings const window_settings = { 800, 600, set_window_callbacks };
    mouse_last.x = (f32) window_settings.width / 2.0f;
    mouse_last.y = (f32) window_settings.height / 2.0f;

    camera = new_camera_at((vec3) { 0, 0, 3 });
    camera.aspect = (f32) window_settings.width / (f32) window_settings.height;

    GLFWwindow *const window = init_opengl(window_settings, &err);
    if (err) { goto main_err; }

    stbi_set_flip_vertically_on_load(true);

    // @Volatile: use these same files in `process_input`.
    shader = TRY_NEW_SHADER("simple_texture", &err);

    Texture const cubes = TRY_NEW_TEXTURE("marble.jpg", &err);
    Texture const floor = TRY_NEW_TEXTURE("metal.png", &err);

    TextureSettings tex_settings = Default_TextureSettings;
    tex_settings.format = TextureFormat_Rgba; // load alpha
    tex_settings.wrap_s = TextureWrap_ClampToEdge;
    tex_settings.wrap_t = TextureWrap_ClampToEdge;
    Texture const transparent = new_texture_from_filepath_with_settings(
        tex_settings, GLOW_TEXTURES_ "blending_transparent_window.png", &err);
    if (err) { goto main_err; }

#define BIND_DATA_TO_VBO_AND_SET_VAO_ATTRIBS(data, vbo, vao)                                 \
    glBindBuffer(GL_ARRAY_BUFFER, vbo);                                                      \
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);                       \
    glBindVertexArray(vao);                                                                  \
    {                                                                                        \
        int const stride = sizeof(f32) * 5;                                                  \
        glEnableVertexAttribArray(0); /* position attribute */                               \
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void *) 0);                 \
        glEnableVertexAttribArray(1); /* texture coords attribute */                         \
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void *) (sizeof(f32) * 3)); \
    }                                                                                        \
    glBindVertexArray(0)

    uint vao_cubes, vao_floor, vao_transparent;
    glGenVertexArrays(1, &vao_cubes);
    glGenVertexArrays(1, &vao_floor);
    glGenVertexArrays(1, &vao_transparent);
    {
        uint vbos[3];
        glGenBuffers(3, vbos);
        BIND_DATA_TO_VBO_AND_SET_VAO_ATTRIBS(CUBE_VERTICES, vbos[0], vao_cubes);
        BIND_DATA_TO_VBO_AND_SET_VAO_ATTRIBS(FLOOR_VERTICES, vbos[1], vao_floor);
        BIND_DATA_TO_VBO_AND_SET_VAO_ATTRIBS(TRANSPARENT_VERTICES, vbos[2], vao_transparent);
        glDeleteBuffers(3, vbos);
    }

#undef BIND_DATA_TO_VBO_AND_SET_VAO_ATTRIBS

    // clang-format off
    vec3 const transparent_positions[] = {
        { -1.5f, 0.0f, -0.48f },
        {  1.5f, 0.0f,  0.51f },
        {  0.0f, 0.0f,  0.7f  },
        { -0.3f, 0.0f, -2.3f  },
        {  0.5f, 0.0f, -0.6f  },
    };
    // clang-format on

    use_shader(shader);
    { set_shader_sampler2D(shader, "texture1", GL_TEXTURE0); }

    f32 last_frame = 0; // time of last frame
    f32 delta_time = 0; // time between consecutive frames

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // @Cleanup
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while (!glfwWindowShouldClose(window)) {
        f32 const curr_frame = glfwGetTime();
        delta_time = curr_frame - last_frame;
        last_frame = curr_frame;
        process_input(window, delta_time);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mat4 const projection = get_camera_projection_matrix(&camera);
        mat4 const view = get_camera_view_matrix(&camera);

        use_shader(shader);
        {
            set_shader_mat4(shader, "world_to_view", view);
            set_shader_mat4(shader, "view_to_clip", projection);

            // Cubes.
            glBindVertexArray(vao_cubes);
            bind_texture_to_unit(cubes, GL_TEXTURE0);
            set_shader_mat4(shader, "local_to_world", mat4_translate((vec3) { -1, 0, -1 }));
            glDrawArrays(GL_TRIANGLES, 0, 36);
            set_shader_mat4(shader, "local_to_world", mat4_translate((vec3) { 2, 0, 0 }));
            glDrawArrays(GL_TRIANGLES, 0, 36);

            // Floor.
            glBindVertexArray(vao_floor);
            bind_texture_to_unit(floor, GL_TEXTURE0);
            set_shader_mat4(shader, "local_to_world", mat4_id());
            glDrawArrays(GL_TRIANGLES, 0, 6);

            // Window.
            glBindVertexArray(vao_transparent);
            bind_texture_to_unit(transparent, GL_TEXTURE0);
            qsort( // sort by view distance before rendering
                (void *) transparent_positions,
                ARRAY_LEN(transparent_positions),
                sizeof(transparent_positions[0]),
                transparent_cmp);
            for (usize i = 0; i < ARRAY_LEN(transparent_positions); ++i) {
                mat4 const model = mat4_translate(transparent_positions[i]);
                set_shader_mat4(shader, "local_to_world", model);
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }

            glBindVertexArray(0);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao_transparent);
    glDeleteVertexArrays(1, &vao_floor);
    glDeleteVertexArrays(1, &vao_cubes);
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
        case Err_Assimp_Import: GLOW_ERROR("aiImportFile() failed"); break;
        case Err_Assimp_Get_Texture: GLOW_ERROR("aiGetMaterialTexture() failed"); break;
        case Err_Fopen: GLOW_ERROR("fopen() failed"); break;
        case Err_Malloc: GLOW_ERROR("malloc() failed"); break;
        case Err_Calloc: GLOW_ERROR("calloc() failed"); break;
        case Err_Realloc: GLOW_ERROR("realloc() failed"); break;
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
        RELOAD_SHADER("simple_texture", &shader);
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
