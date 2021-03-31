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
bool is_b_pressed = false;
bool use_blinn_phong = false;

Shader cube_shader;
Shader light_cube_shader;

// Forward declarations.
void process_input(GLFWwindow *window, f32 const delta_time);
void set_window_callbacks(GLFWwindow *window);

int main(int argc, char *argv[]) {
    Err err = Err_None;

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

        -0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,    0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,    1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,    1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,    1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,    0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,    0.0f, 0.0f,

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

    vec3 const point_light_positions[] = {
        {  0.7f,  0.2f,   2.0f },
        {  2.3f, -3.3f,  -4.0f },
        { -4.0f,  2.0f, -12.0f },
        {  0.0f,  0.0f,  -3.0f },
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
        glEnableVertexAttribArray(0); // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void *) 0);

        glEnableVertexAttribArray(1); // normal attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void *) (sizeof(f32) * 3));

        glEnableVertexAttribArray(2); // texcoord attribute
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void *) (sizeof(f32) * 6));
    }
    glBindVertexArray(vao_light_cube);
    {
        glEnableVertexAttribArray(0); // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void *) 0);
    }
    glBindVertexArray(0);

    use_shader(cube_shader);
    {
        set_shader_sampler2D(cube_shader, "material.diffuse", GL_TEXTURE0);
        set_shader_sampler2D(cube_shader, "material.specular", GL_TEXTURE1);
        set_shader_float(cube_shader, "material.shininess", 32.0f);
    }

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

        mat4 const projection = get_camera_projection_matrix(&camera);
        mat4 const view = get_camera_view_matrix(&camera);

        // clang-format off
        use_shader(cube_shader);
        {
            { // Directional light.
                set_shader_vec3(
                    cube_shader, "directional_light.direction", (vec3) { -0.2f, -1.0f, -0.3f });

                set_shader_vec3(cube_shader, "directional_light.k.ambient", vec3_of(0.05f));
                set_shader_vec3(cube_shader, "directional_light.k.diffuse", vec3_of(0.4f));
                set_shader_vec3(cube_shader, "directional_light.k.specular", vec3_of(0.5f));
            }

            { // Point lights.
                #define POINT_LIGHT(i) "point_lights[" STRINGIFY(i) "]"

                #define SET_SHADER_POINT_LIGHT(i)                                              \
                    assert((i) < ARRAY_LEN(point_light_positions));                            \
                                                                                               \
                    set_shader_vec3(                                                           \
                        cube_shader, POINT_LIGHT(i) ".position", point_light_positions[(i)]);  \
                                                                                               \
                    set_shader_vec3(cube_shader, POINT_LIGHT(i) ".k.ambient", vec3_of(0.05f)); \
                    set_shader_vec3(cube_shader, POINT_LIGHT(i) ".k.diffuse", vec3_of(0.8f));  \
                    set_shader_vec3(cube_shader, POINT_LIGHT(i) ".k.specular", vec3_of(1.0f)); \
                                                                                               \
                    set_shader_float(cube_shader, POINT_LIGHT(i) ".att.constant", 1.0f);       \
                    set_shader_float(cube_shader, POINT_LIGHT(i) ".att.linear", 0.09);         \
                    set_shader_float(cube_shader, POINT_LIGHT(i) ".att.quadratic", 0.032);

                SET_SHADER_POINT_LIGHT(0);
                SET_SHADER_POINT_LIGHT(1);
                SET_SHADER_POINT_LIGHT(2);
                SET_SHADER_POINT_LIGHT(3);

                #undef SET_SHADER_POINT_LIGHT
                #undef POINT_LIGHT
            }

            { // Spot light.
                set_shader_vec3(cube_shader, "spot_light.position", camera.position);
                set_shader_vec3(cube_shader, "spot_light.direction", camera.forward);

                set_shader_float(
                    cube_shader, "spot_light.cos_cutoff_inner", cosf(RADIANS(12.5f)));
                set_shader_float(
                    cube_shader, "spot_light.cos_cutoff_outer", cosf(RADIANS(15.0f)));

                set_shader_vec3(cube_shader, "spot_light.k.ambient", vec3_of(0.0f));
                set_shader_vec3(cube_shader, "spot_light.k.diffuse", vec3_of(1.0f));
                set_shader_vec3(cube_shader, "spot_light.k.specular", vec3_of(1.0f));

                set_shader_float(cube_shader, "spot_light.att.constant", 1.0f);
                set_shader_float(cube_shader, "spot_light.att.linear", 0.09);
                set_shader_float(cube_shader, "spot_light.att.quadratic", 0.032);
            }
        }
        // clang-format on

        use_shader(cube_shader);
        {
            set_shader_bool(cube_shader, "use_blinn_phong", use_blinn_phong);
            set_shader_vec3(cube_shader, "view_pos", camera.position);

            set_shader_mat4(cube_shader, "world_to_view", view);
            set_shader_mat4(cube_shader, "view_to_clip", projection);

            bind_texture_to_unit(diffuse_map, GL_TEXTURE0);
            bind_texture_to_unit(specular_map, GL_TEXTURE1);

            glBindVertexArray(vao_cube);
            for (int i = 0; i < ARRAY_LEN(cube_positions); ++i) {
                f32 const angle = RADIANS((i % 3) ? -20 * glfwGetTime() : 25 * glfwGetTime());
                mat4 const model = mat4_mul(
                    mat4_translate(cube_positions[i]),
                    mat4_rotate(angle, (vec3) { 1.0f, 0.3f, 0.5f }));
                set_shader_mat4(cube_shader, "local_to_world", model);

                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
        }

        use_shader(light_cube_shader);
        {
            set_shader_mat4(light_cube_shader, "world_to_view", view);
            set_shader_mat4(light_cube_shader, "view_to_clip", projection);

            glBindVertexArray(vao_light_cube);
            for (int i = 0; i < ARRAY_LEN(point_light_positions); ++i) {
                mat4 const model =
                    mat4_mul(mat4_translate(point_light_positions[i]), mat4_scale(vec3_of(0.2f)));
                set_shader_mat4(light_cube_shader, "local_to_world", model);

                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
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

    if ON_PRESS (B, is_b_pressed) {
        if ((use_blinn_phong = !use_blinn_phong)) {
            GLOW_LOG("Using Blinn-Phong shading");
        } else {
            GLOW_LOG("Using Phong shading");
        }
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
