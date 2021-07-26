#include "main.h"

#define USE_SHADOW_MAPPING 1
#define SHADOW_MAP_RESOLUTION 1024

#define USE_INSTANCED_RENDERING 1

static bool is_tab_pressed = false;
static bool mouse_is_first = true;
static vec2 mouse_last = { 0 };
static Clock clock = { 0 };
static Fps fps = { 0 };

static Camera camera;

static PathsToShader skybox;
static PathsToShader debug_quad;
static PathsToShader shadow_mapping;

static Texture skybox_texture;
static Texture wood_texture;

typedef struct Resources {
    uint vao_skybox;

    vec3 light_position;
    uint vao_plane;

    uint fbo_depth_map;
    uint tex_depth_map;
} Resources;

static inline Resources create_resources(Err *err, int width, int height);
static inline void destroy_resources(Resources *r, int width, int height);
static inline void draw_frame(Resources const *r, int width, int height);

int main(int argc, char *argv[]) {
    Err err = Err_None;

    WindowSettings const window_settings = { 800, 600, set_window_callbacks, .msaa = 4 };

    GLFWwindow *window = init_opengl(window_settings, &err);
    if (err) { goto main_exit_opengl; }

    int w = 0, h = 0;
    glfwGetWindowSize(window, &w, &h);
    assert(w > 0 && h > 0);

    mouse_last.x = 0.5f * w;
    mouse_last.y = 0.5f * h;

    camera = new_camera_at((vec3) { 0, 0, 55 });
    camera.aspect = (f32) w / (f32) h;
    camera.movement_speed = 50.0f;
    camera.far = 1000.0f;

    Resources r = create_resources(&err, w, h);
    if (err) { goto main_exit; }

    setup_shaders();

    while (!glfwWindowShouldClose(window)) {
        glfwGetWindowSize(window, &w, &h);
        begin_frame(window, w, h);
        draw_frame(&r, w, h);
        end_frame(window, w, h);
    }

    assert(err == Err_None);

main_exit:
    destroy_resources(&r, w, h);

main_exit_opengl:
    glfwDestroyWindow(window);
    glfwTerminate();

    switch (err) {
        case Err_None: break;
        case Err_Unkown: GLOW_ERROR("unkown error"); break;
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

    return err ? EXIT_FAILURE : EXIT_SUCCESS;
}

//
// Resources setup and cleanup.
//

static inline Resources create_resources(Err *err, int width, int height) {
    Resources r = { 0 };

    skybox.paths.vertex = GLOW_SHADERS_ "simple_skybox.vs";
    skybox.paths.fragment = GLOW_SHADERS_ "simple_skybox.fs";

    debug_quad.paths.vertex = GLOW_SHADERS_ "debug_quad_depth.vs";
    debug_quad.paths.fragment = GLOW_SHADERS_ "debug_quad_depth.fs";

    shadow_mapping.paths.vertex = GLOW_SHADERS_ "shadow_mapping_depth.vs";
    shadow_mapping.paths.fragment = GLOW_SHADERS_ "shadow_mapping_depth.fs";

    skybox.shader = new_shader_from_filepath(skybox.paths, err);
    debug_quad.shader = new_shader_from_filepath(debug_quad.paths, err);
    shadow_mapping.shader = new_shader_from_filepath(shadow_mapping.paths, err);

    stbi_set_flip_vertically_on_load(false);
    skybox_texture = new_cubemap_texture_from_filepaths(
        (char const *[6]) {
            GLOW_TEXTURES_ "skybox/right.jpg", // +X
            GLOW_TEXTURES_ "skybox/left.jpg", // -X
            GLOW_TEXTURES_ "skybox/top.jpg", // +Y
            GLOW_TEXTURES_ "skybox/bottom.jpg", // -Y
            GLOW_TEXTURES_ "skybox/front.jpg", // +Z
            GLOW_TEXTURES_ "skybox/back.jpg", // -Z
        },
        err);

    stbi_set_flip_vertically_on_load(true);
    wood_texture = new_texture_from_filepath(GLOW_TEXTURES_ "wood.png", err);

    // Exit early if there were any errors during setup.
    if (*err != Err_None) { return r; }

    //
    // Skybox vertices (vao_skybox).
    //

    glGenVertexArrays(1, &r.vao_skybox);
    {
        uint vbo;
        glGenBuffers(1, &vbo);
        DEFER(glDeleteBuffers(1, &vbo)) {
            glBindVertexArray(r.vao_skybox);
            DEFER(glBindVertexArray(0)) {
                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                glBufferData(
                    GL_ARRAY_BUFFER, sizeof(SKYBOX_VERTICES), SKYBOX_VERTICES, GL_STATIC_DRAW);

                glEnableVertexAttribArray(0); // position
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(f32) * 3, (void *) 0);
            }
        }
    }

    //
    // Scene description (vao_plane, light_position).
    //

    r.light_position = (vec3) { -2.0f, 4.0f, -1.0f };

    glGenVertexArrays(1, &r.vao_plane);
    {
        uint vbo;
        glGenBuffers(1, &vbo);
        DEFER(glDeleteBuffers(1, &vbo)) {
            glBindVertexArray(r.vao_plane);
            DEFER(glBindVertexArray(0)) {
                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                glBufferData(
                    GL_ARRAY_BUFFER, sizeof(PLANE_VERTICES), PLANE_VERTICES, GL_STATIC_DRAW);

                glEnableVertexAttribArray(0); // position
                glEnableVertexAttribArray(1); // normal
                glEnableVertexAttribArray(2); // texcoord

                glVertexAttribPointer(
                    0, 3, GL_FLOAT, GL_FALSE, sizeof(f32) * 8, (void *) (sizeof(f32) * 0));
                glVertexAttribPointer(
                    1, 3, GL_FLOAT, GL_FALSE, sizeof(f32) * 8, (void *) (sizeof(f32) * 3));
                glVertexAttribPointer(
                    2, 3, GL_FLOAT, GL_FALSE, sizeof(f32) * 8, (void *) (sizeof(f32) * 6));
            }
        }
    }

    //
    // Shadow mapping (fbo_depth_map).
    //

#if USE_SHADOW_MAPPING
    // Create a depth texture to be rendered from the lights' point of view.
    glGenTextures(1, &r.tex_depth_map);
    glBindTexture(GL_TEXTURE_2D, r.tex_depth_map);
    DEFER(glBindTexture(GL_TEXTURE_2D, 0)) {
        glTexImage2D(
            /*target*/ GL_TEXTURE_2D,
            /*level*/ 0,
            /*internalFormat*/ GL_DEPTH_COMPONENT,
            /*width*/ SHADOW_MAP_RESOLUTION,
            /*height*/ SHADOW_MAP_RESOLUTION,
            /*border*/ 0,
            /*format*/ GL_DEPTH_COMPONENT,
            /*type*/ GL_FLOAT,
            /*data*/ NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    // Attach it to the fbo's depth buffer.
    glGenFramebuffers(1, &r.fbo_depth_map);
    glBindFramebuffer(GL_FRAMEBUFFER, r.fbo_depth_map);
    DEFER(glBindFramebuffer(GL_FRAMEBUFFER, 0)) {
        glFramebufferTexture2D(
            /*target*/ GL_FRAMEBUFFER,
            /*attachment*/ GL_DEPTH_ATTACHMENT,
            /*textarget*/ GL_TEXTURE_2D,
            /*texture*/ r.tex_depth_map,
            /*level*/ 0);

        // Specify no color buffer for rendering.
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        // Check if the framebuffer is complete.
        int const status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            GLOW_WARNING("framebuffer is incomplete, status: `0x%x`", status);
        }
    }
#endif

    return r;
}

static inline void destroy_resources(Resources *r, int width, int height) {
    UNUSED(width);
    UNUSED(height);

#if USE_SHADOW_MAPPING
    glDeleteFramebuffers(1, &r->fbo_depth_map);
#endif
    glDeleteVertexArrays(1, &r->vao_plane);
    glDeleteVertexArrays(1, &r->vao_skybox);

    glDeleteTextures(1, &wood_texture.id);
    glDeleteTextures(1, &skybox_texture.id);

    glDeleteProgram(shadow_mapping.shader.program_id);
    glDeleteProgram(debug_quad.shader.program_id);
    glDeleteProgram(skybox.shader.program_id);
}

//
// Frame rendering pre- and post-processing.
//

static inline void begin_frame(GLFWwindow *window, int width, int height) {
    UNUSED(width);
    UNUSED(height);

    clock_tick(&clock, glfwGetTime());
    update_frame_counter(&fps, clock.time);
    process_input(window, clock.time_increment);

    if (fps.last_update_time == clock.time) {
        char title[32]; // 32 seems large enough..
        snprintf(title, sizeof(title), "glow | %d fps", fps.rate);
        glfwSetWindowTitle(window, title);
    }
}

static inline void end_frame(GLFWwindow *window, int width, int height) {
    UNUSED(width);
    UNUSED(height);

    glfwSwapBuffers(window);
    glfwPollEvents();
}

//
// Frame rendering.
//

static inline void draw_frame(Resources const *r, int width, int height) {
    mat4 const projection = get_camera_projection_matrix(&camera);
    mat4 const view = get_camera_view_matrix(&camera);

#if USE_SHADOW_MAPPING
    // @Note: first render to depth map, then render the scene as normal
    // with shadow mapping (by using the depth map).

    glViewport(0, 0, SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION);
    DEFER(glViewport(0, 0, width, height)) {
        glBindFramebuffer(GL_FRAMEBUFFER, r->fbo_depth_map);
        DEFER(glBindFramebuffer(GL_FRAMEBUFFER, 0)) {
            glClear(GL_DEPTH_BUFFER_BIT);

            use_shader(shadow_mapping.shader);
            {
                // Use an orthographic projection matrix to model a directional light source
                // (i.e. all its rays are parallel), so there is no perspective deform.
                mat4 const light_projection =
                    mat4_ortho(-10, 10, -10, 10, /*near*/ 1.0f, /*far*/ 7.5f);

                mat4 const light_view =
                    mat4_lookat(r->light_position, /*target*/ (vec3) { 0 }, (vec3) { 0, 1, 0 });

                set_shader_mat4(
                    shadow_mapping.shader,
                    "world_to_light_space",
                    mat4_mul(light_projection, light_view));
            }

            // @Todo: render the scene.
        }
    }
#endif

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#if USE_SHADOW_MAPPING
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, r->tex_depth_map);
#endif

    // Change the depth function to make sure the skybox passes the depth tests.
    glDepthFunc(GL_LEQUAL);
    use_shader(skybox.shader);
    {
        mat4 view_without_translation = view;
        view_without_translation.m[0][3] = 0.0f;
        view_without_translation.m[1][3] = 0.0f;
        view_without_translation.m[2][3] = 0.0f;
        set_shader_mat4(skybox.shader, "world_to_view", view_without_translation);
        set_shader_mat4(skybox.shader, "view_to_clip", projection);

        glBindVertexArray(r->vao_skybox);
        DEFER(glBindVertexArray(0)) {
            bind_texture_to_unit(skybox_texture, GL_TEXTURE0);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }
    glDepthFunc(GL_LESS);
}

//
// Shaders' first run setup.
//

static inline void setup_shaders(void) {
    use_shader(debug_quad.shader);
    set_shader_sampler2D(debug_quad.shader, "depth_map", GL_TEXTURE0);

    use_shader(skybox.shader);
    set_shader_sampler2D(skybox.shader, "skybox", GL_TEXTURE0);
}

//
// Input processing.
//

#define IS_PRESSED(key) (glfwGetKey(window, GLFW_KEY_##key) == GLFW_PRESS)

#define ON_PRESS(key, is_key_pressed)                                              \
    (glfwGetKey(window, GLFW_KEY_##key) != GLFW_PRESS) { is_key_pressed = false; } \
    else if (!is_key_pressed && (is_key_pressed = true))

static inline void process_input(GLFWwindow *window, f32 delta_time) {
    if IS_PRESSED (ESCAPE) { glfwSetWindowShouldClose(window, true); }

    if IS_PRESSED (W) { update_camera_position(&camera, CameraMovement_Forward, delta_time); }
    if IS_PRESSED (S) { update_camera_position(&camera, CameraMovement_Backward, delta_time); }
    if IS_PRESSED (A) { update_camera_position(&camera, CameraMovement_Left, delta_time); }
    if IS_PRESSED (D) { update_camera_position(&camera, CameraMovement_Right, delta_time); }
    if IS_PRESSED (E) { update_camera_position(&camera, CameraMovement_Up, delta_time); }
    if IS_PRESSED (Q) { update_camera_position(&camera, CameraMovement_Down, delta_time); }

    if ON_PRESS (TAB, is_tab_pressed) {
        GLOW_LOG("Hot swapping shaders");

        reload_shader_from_filepath(&skybox.shader, skybox.paths);
        reload_shader_from_filepath(&debug_quad.shader, debug_quad.paths);
        reload_shader_from_filepath(&shadow_mapping.shader, shadow_mapping.paths);

        setup_shaders();
    }
}

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

static void set_window_callbacks(GLFWwindow *window) {
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
}
