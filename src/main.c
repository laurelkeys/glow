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
bool is_b_pressed = false;
bool use_blinn_phong = false;

Shader light_cube_shader;
Shader backpack_shader;
Shader outline_shader;

vec3 point_lights[4];

// Forward declarations.
void process_input(GLFWwindow *window, f32 delta_time);
void set_window_callbacks(GLFWwindow *window);
void setup_blinn_phong_shader_lights(Shader const shader);

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
    light_cube_shader = TRY_NEW_SHADER("simple_white", &err);
    backpack_shader = TRY_NEW_SHADER("blinn_phong", &err);
    outline_shader = TRY_NEW_SHADER("single_color", &err);

    Texture const diffuse_map = TRY_NEW_TEXTURE("container2.png", &err);
    Texture const specular_map = TRY_NEW_TEXTURE("container2_specular.png", &err);

    Model backpack = TRY_ALLOC_NEW_MODEL("backpack/backpack.obj", &err);

    // clang-format off
    point_lights[0] = (vec3) {  0.7f,  0.2f,   2.0f };
    point_lights[1] = (vec3) {  2.3f, -3.3f,  -4.0f };
    point_lights[2] = (vec3) { -4.0f,  2.0f, -12.0f };
    point_lights[3] = (vec3) {  0.0f,  0.0f,  -3.0f };
    // clang-format on

    uint vao_light_cube;
    glGenVertexArrays(1, &vao_light_cube);
    {
        uint vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(CUBE_VERTICES), CUBE_VERTICES, GL_STATIC_DRAW);

        glBindVertexArray(vao_light_cube);
        {
            glEnableVertexAttribArray(0); // position attribute
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(f32) * 8, (void *) 0);
        }
        glBindVertexArray(0);

        glDeleteBuffers(1, &vbo);
    }

    f32 last_frame = 0; // time of last frame
    f32 delta_time = 0; // time between consecutive frames

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // @Cleanup
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glStencilOp(/*sfail*/ GL_KEEP, /*dpfail*/ GL_KEEP, /*dppass*/ GL_REPLACE);

    while (!glfwWindowShouldClose(window)) {
        f32 const curr_frame = glfwGetTime();
        delta_time = curr_frame - last_frame;
        last_frame = curr_frame;
        process_input(window, delta_time);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        mat4 const projection = get_camera_projection_matrix(&camera);
        mat4 const view = get_camera_view_matrix(&camera);

        //
        // Backpack model shader.
        //

        // Add 1 to the stencil buffer wherever the object's fragments are rendered.
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);

        use_shader(backpack_shader);
        {
            set_shader_mat4(backpack_shader, "local_to_world", mat4_id());
            set_shader_mat4(backpack_shader, "world_to_view", view);
            set_shader_mat4(backpack_shader, "view_to_clip", projection);

            setup_blinn_phong_shader_lights(backpack_shader);
            set_shader_bool(backpack_shader, "use_blinn_phong", use_blinn_phong);
            set_shader_vec3(backpack_shader, "view_pos", camera.position);

            set_shader_float(backpack_shader, "material.shininess", 32.0f);

            draw_model_with_shader(&backpack, backpack_shader);
        }

        //
        // Outline color shader.
        //

        // Disable depth testing and writing to the stencil buffer, and only draw
        // where the stencil buffer is not equal to 1 (to outline the object).
        glDisable(GL_DEPTH_TEST);
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00);

        use_shader(outline_shader);
        {
            // Render the same backpack model, but slightly upscaled.
            set_shader_mat4(outline_shader, "local_to_world", mat4_scale(vec3_of(1.02f)));
            set_shader_mat4(outline_shader, "world_to_view", view);
            set_shader_mat4(outline_shader, "view_to_clip", projection);

            vec3 const outline_color = { 0.04f, 0.28f, 0.26f };
            set_shader_vec3(outline_shader, "color", outline_color);

            // @Hack: draw "textureless" as we don't use the materials.
            draw_textureless_model_with_shader(&backpack, outline_shader);
        }

        //
        // Light cube shader.
        //

        // Re-enable depth testing and writing to the stencil buffer.
        glEnable(GL_DEPTH_TEST);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);

        use_shader(light_cube_shader);
        {
            set_shader_mat4(light_cube_shader, "world_to_view", view);
            set_shader_mat4(light_cube_shader, "view_to_clip", projection);

            glBindVertexArray(vao_light_cube);
            for (usize i = 0; i < ARRAY_LEN(point_lights); ++i) {
                set_shader_mat4(
                    light_cube_shader,
                    "local_to_world",
                    mat4_mul(mat4_translate(point_lights[i]), mat4_scale(vec3_of(0.2f))));

                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
            glBindVertexArray(0);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao_light_cube);
    dealloc_model(&backpack);
    glDeleteProgram(outline_shader.program_id);
    glDeleteProgram(backpack_shader.program_id);
    glDeleteProgram(light_cube_shader.program_id);

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

void setup_blinn_phong_shader_lights(Shader const shader) {
    //
    // Directional light.
    //

    set_shader_vec3(
        backpack_shader, "directional_light.direction", (vec3) { -0.2f, -1.0f, -0.3f });

    set_shader_vec3(backpack_shader, "directional_light.k.ambient", vec3_of(0.05f));
    set_shader_vec3(backpack_shader, "directional_light.k.diffuse", vec3_of(0.4f));
    set_shader_vec3(backpack_shader, "directional_light.k.specular", vec3_of(0.5f));

    //
    // Point lights.
    //

    assert(ARRAY_LEN(point_lights) == 4);

#define POINT_LIGHT(i) "point_lights[" STRINGIFY(i) "]"

#define SET_SHADER_POINT_LIGHT(i)                                                    \
                                                                                     \
    set_shader_vec3(backpack_shader, POINT_LIGHT(i) ".position", point_lights[(i)]); \
                                                                                     \
    set_shader_vec3(backpack_shader, POINT_LIGHT(i) ".k.ambient", vec3_of(0.05f));   \
    set_shader_vec3(backpack_shader, POINT_LIGHT(i) ".k.diffuse", vec3_of(0.8f));    \
    set_shader_vec3(backpack_shader, POINT_LIGHT(i) ".k.specular", vec3_of(1.0f));   \
                                                                                     \
    set_shader_float(backpack_shader, POINT_LIGHT(i) ".att.constant", 1.0f);         \
    set_shader_float(backpack_shader, POINT_LIGHT(i) ".att.linear", 0.09);           \
    set_shader_float(backpack_shader, POINT_LIGHT(i) ".att.quadratic", 0.032);

    SET_SHADER_POINT_LIGHT(0);
    SET_SHADER_POINT_LIGHT(1);
    SET_SHADER_POINT_LIGHT(2);
    SET_SHADER_POINT_LIGHT(3);

#undef SET_SHADER_POINT_LIGHT
#undef POINT_LIGHT

    //
    // Spot light.
    //

    set_shader_vec3(backpack_shader, "spot_light.position", camera.position);
    set_shader_vec3(backpack_shader, "spot_light.direction", camera.forward);

    set_shader_float(backpack_shader, "spot_light.cos_cutoff_inner", cosf(RADIANS(12.5f)));
    set_shader_float(backpack_shader, "spot_light.cos_cutoff_outer", cosf(RADIANS(15.0f)));

    set_shader_vec3(backpack_shader, "spot_light.k.ambient", vec3_of(0.0f));
    set_shader_vec3(backpack_shader, "spot_light.k.diffuse", vec3_of(1.0f));
    set_shader_vec3(backpack_shader, "spot_light.k.specular", vec3_of(1.0f));

    set_shader_float(backpack_shader, "spot_light.att.constant", 1.0f);
    set_shader_float(backpack_shader, "spot_light.att.linear", 0.09);
    set_shader_float(backpack_shader, "spot_light.att.quadratic", 0.032);
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
        RELOAD_SHADER("simple_white", &light_cube_shader);
        RELOAD_SHADER("blinn_phong", &backpack_shader);
        RELOAD_SHADER("single_color", &outline_shader);
    }

    if ON_PRESS (B, is_b_pressed) {
        if ((use_blinn_phong = !use_blinn_phong)) {
            GLOW_LOG("Using Blinn-Phong shading");
        } else {
            GLOW_LOG("Using Phong shading");
        }
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
