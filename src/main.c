#include "main.h"

#define USE_MSAA 1
#define MSAA_SAMPLES 4

#define USE_USE_SHADOW_MAPPING 1
#define SHADOW_MAP_RESOLUTION 1024

#define USE_INSTANCED_RENDERING 1

#define RANDOM(a, b) ((a) + ((b) - (a)) * ((f32) rand() / (f32) RAND_MAX))

static bool is_tab_pressed = false;
static bool mouse_is_first = true;
static vec2 mouse_last = { 0 };
static Clock clock = { 0 };
static Fps fps = { 0 };

static Camera camera;

static PathsToShader skybox;
static PathsToShader planet;
static PathsToShader rock;

static Texture skybox_texture;

static Model planet_model;
static Model rock_model;

typedef struct Resources {
    uint vao_skybox;

    uint fbo_msaa;

    uint fbo_depth_map;

    usize instance_count;
    mat4 *instance_model_matrices;
} Resources;

static inline Resources create_resources(Err *err, int width, int height);
static inline void destroy_resources(Resources *r, int width, int height);

static inline void begin_frame(GLFWwindow *window, int width, int height);
static inline void draw_frame(Resources const *r, int width, int height);
static inline void end_frame(GLFWwindow *window, int width, int height);

int main(int argc, char *argv[]) {
    Err err = Err_None;

    WindowSettings const window_settings = { 800, 600, set_window_callbacks };

    GLFWwindow *window = init_opengl(window_settings, &err);
    if (err) { goto main_exit_opengl; }

    int w, h;
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
    skybox.paths.vertex = GLOW_SHADERS_ "simple_skybox.vs";
    skybox.paths.fragment = GLOW_SHADERS_ "simple_skybox.fs";

    planet.paths.vertex = GLOW_SHADERS_ "instancing_texture_test.vs";
    planet.paths.fragment = GLOW_SHADERS_ "instancing_texture_test.fs";

#if USE_INSTANCED_RENDERING
    rock.paths.vertex = GLOW_SHADERS_ "instancing_texture_test2.vs";
#else
    rock.paths.vertex = GLOW_SHADERS_ "instancing_texture_test.vs";
#endif
    rock.paths.fragment = GLOW_SHADERS_ "instancing_texture_test.fs";

    skybox.shader = new_shader_from_filepath(skybox.paths, err);
    planet.shader = new_shader_from_filepath(planet.paths, err);
    rock.shader = new_shader_from_filepath(rock.paths, err);

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

    stbi_set_flip_vertically_on_load(choose_model[ROCK].flip_on_load);
    rock_model = alloc_new_model_from_filepath(choose_model[ROCK].path, err);

    stbi_set_flip_vertically_on_load(choose_model[PLANET].flip_on_load);
    planet_model = alloc_new_model_from_filepath(choose_model[PLANET].path, err);

    Resources r = { 0 };

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
    // MSAA framebuffer (fbo_msaa).
    //

#if USE_MSAA
    glGenFramebuffers(1, &r.fbo_msaa);
    glBindFramebuffer(GL_FRAMEBUFFER, r.fbo_msaa);
    DEFER(glBindFramebuffer(GL_FRAMEBUFFER, 0)) {
        // Create a multisample texture for the color attachment.
        uint tex_ms;
        glGenTextures(1, &tex_ms);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex_ms);
        DEFER(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0)) {
            glTexImage2DMultisample(
                /*target*/ GL_TEXTURE_2D_MULTISAMPLE,
                /*samples*/ MSAA_SAMPLES,
                /*internalformat*/ GL_RGB,
                /*width*/ width,
                /*height*/ height,
                /*fixedsamplelocations*/ GL_TRUE);
        }

        // Create a multisampled renderbuffer object for depth and stencil attachments.
        uint rbo_ms;
        glGenRenderbuffers(1, &rbo_ms);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo_ms);
        DEFER(glBindRenderbuffer(GL_RENDERBUFFER, 0)) {
            glRenderbufferStorageMultisample(
                /*target*/ GL_RENDERBUFFER,
                /*samples*/ MSAA_SAMPLES,
                /*internalformat*/ GL_DEPTH24_STENCIL8,
                /*width*/ width,
                /*height*/ height);
        }

        // Attach the texture and renderbuffer to the framebuffer.
        glFramebufferTexture2D(
            /*target*/ GL_FRAMEBUFFER,
            /*attachment*/ GL_COLOR_ATTACHMENT0,
            /*textarget*/ GL_TEXTURE_2D_MULTISAMPLE,
            /*texture*/ tex_ms,
            /*level*/ 0);
        glFramebufferRenderbuffer(
            /*target*/ GL_FRAMEBUFFER,
            /*attachment*/ GL_DEPTH_STENCIL_ATTACHMENT,
            /*renderbuffertarget*/ GL_RENDERBUFFER,
            /*renderbuffer*/ rbo_ms);

        // Check if the framebuffer is complete.
        int const status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            GLOW_WARNING("framebuffer is incomplete, status: `0x%x`", status);
        }
    }
#endif

    //
    // Shadow mapping framebuffer (fbo_depth_map).
    //

#if USE_USE_SHADOW_MAPPING
    glGenFramebuffers(1, &r.fbo_depth_map);
    glBindFramebuffer(GL_FRAMEBUFFER, r.fbo_depth_map);
    DEFER(glBindFramebuffer(GL_FRAMEBUFFER, 0)) {
        // Create a depth texture to be rendered from the lights' point of view.
        uint tex_depth_map;
        glGenTextures(1, &tex_depth_map);
        glBindTexture(GL_TEXTURE_2D, tex_depth_map);
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

        // Attach it to the fbo's depth buffer and specify no color buffer for rendering.
        glFramebufferTexture2D(
            /*target*/ GL_FRAMEBUFFER,
            /*attachment*/ GL_DEPTH_ATTACHMENT,
            /*textarget*/ GL_TEXTURE_2D,
            /*texture*/ tex_depth_map,
            /*level*/ 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        // Check if the framebuffer is complete.
        int const status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            GLOW_WARNING("framebuffer is incomplete, status: `0x%x`", status);
        }
    }
#endif

    //
    // Instanced rendering (instance_count, instance_model_matrices).
    //

    f32 const offset = 2.5f; // 25.0f;
    f32 const radius = 50.0f; // 150.0f;
    r.instance_count = 10000; // 100000;
    r.instance_model_matrices = calloc(r.instance_count, sizeof(mat4));

    srand((uint) glfwGetTime()); // initialize random seed
    for (usize i = 0; i < r.instance_count; ++i) {
        f32 const rotation_angle = RANDOM(0.0f, 360.0f);
        f32 const translation_angle = ((f32) i / (f32) r.instance_count) * 360.0f;

        vec3 const displacement = {
            .x = RANDOM(-offset, offset) + sinf(translation_angle) * radius,
            .y = RANDOM(-offset, offset) * 0.4f,
            .z = RANDOM(-offset, offset) + cosf(translation_angle) * radius,
        };

        mat4 const T = mat4_translate(displacement);
        mat4 const S = mat4_scale(vec3_of(RANDOM(0.05f, 0.25f)));
        mat4 const R = mat4_rotate(rotation_angle, (vec3) { 0.4f, 0.6f, 0.8f });

        r.instance_model_matrices[i] = mat4_mul(T, mat4_mul(S, R));
    }

#if USE_INSTANCED_RENDERING
    // Use instanced arrays instead of passing the model matrix values as uniforms to the shader.
    // @Note: these are defined as a vertex attribute (allowing us to store much more data) that
    // are updated per instance (with `divisor` = 1) instead of per vertex (with `divisor` = 0).

    uint vbo_instance;
    glGenBuffers(1, &vbo_instance);
    DEFER(glDeleteBuffers(1, &vbo_instance)) {
        DEFER(glBindVertexArray(0)) {
            glBindBuffer(GL_ARRAY_BUFFER, vbo_instance);
            glBufferData(
                GL_ARRAY_BUFFER,
                sizeof(mat4) * r.instance_count,
                &r.instance_model_matrices[0],
                GL_STATIC_DRAW);

            for (usize i = 0; i < rock_model.meshes_len; ++i) {
                glBindVertexArray(rock_model.meshes[i].vao);

                // Set attribute pointers for the model matrix (mat4 = vec4 x 4).
                glEnableVertexAttribArray(3);
                glEnableVertexAttribArray(4);
                glEnableVertexAttribArray(5);
                glEnableVertexAttribArray(6);

                glVertexAttribPointer(
                    3, 4, GL_FLOAT, GL_FALSE, sizeof(mat4), (void *) (sizeof(vec4) * 0));
                glVertexAttribPointer(
                    4, 4, GL_FLOAT, GL_FALSE, sizeof(mat4), (void *) (sizeof(vec4) * 1));
                glVertexAttribPointer(
                    5, 4, GL_FLOAT, GL_FALSE, sizeof(mat4), (void *) (sizeof(vec4) * 2));
                glVertexAttribPointer(
                    6, 4, GL_FLOAT, GL_FALSE, sizeof(mat4), (void *) (sizeof(vec4) * 3));

                glVertexAttribDivisor(3, /*divisor*/ 1);
                glVertexAttribDivisor(4, /*divisor*/ 1);
                glVertexAttribDivisor(5, /*divisor*/ 1);
                glVertexAttribDivisor(6, /*divisor*/ 1);
            }
        }
    }
#endif

    return r;
}

static inline void destroy_resources(Resources *r, int width, int height) {
    UNUSED(width);
    UNUSED(height);

    free(r->instance_model_matrices);

#if USE_USE_SHADOW_MAPPING
    glDeleteFramebuffers(1, &r->fbo_depth_map);
#endif
#if USE_MSAA
    glDeleteFramebuffers(1, &r->fbo_msaa);
#endif
    glDeleteVertexArrays(1, &r->vao_skybox);

    dealloc_model(&rock_model);
    dealloc_model(&planet_model);

    glDeleteProgram(rock.shader.program_id);
    glDeleteProgram(planet.shader.program_id);
    glDeleteProgram(skybox.shader.program_id);
}

//
// Frame rendering, with pre- and post-processing.
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

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static inline void end_frame(GLFWwindow *window, int width, int height) {
    UNUSED(width);
    UNUSED(height);

    glfwSwapBuffers(window);
    glfwPollEvents();
}

static inline void draw_frame(Resources const *r, int width, int height) {
    mat4 const projection = get_camera_projection_matrix(&camera);
    mat4 const view = get_camera_view_matrix(&camera);

#if USE_MSAA
    // Draw the scene to the multisampled framebuffer.
    glBindFramebuffer(GL_FRAMEBUFFER, r->fbo_msaa);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
#endif

    use_shader(planet.shader);
    {
        mat4 const model =
            mat4_mul(mat4_translate((vec3) { 0.0f, -3.0f, 0.0f }), mat4_scale(vec3_of(4.0f)));
        set_shader_mat4(planet.shader, "local_to_world", model);
        set_shader_mat4(planet.shader, "world_to_view", view);
        set_shader_mat4(planet.shader, "view_to_clip", projection);

        draw_model_with_shader(&planet_model, &planet.shader);
    }

    use_shader(rock.shader);
    {
        set_shader_mat4(rock.shader, "world_to_view", view);
        set_shader_mat4(rock.shader, "view_to_clip", projection);

#if USE_INSTANCED_RENDERING
        // @Hack: we know there's a single texture (which is actually a normal, not diffuse).
        assert(rock_model.meshes_len == 1 && rock_model.meshes[0].textures_len == 1);

        set_shader_sampler2D(rock.shader, "texture_diffuse", GL_TEXTURE0);
        bind_texture_to_unit(rock_model.meshes[0].textures[0], GL_TEXTURE0);

        // @Note: this is pretty much `draw_model_with_shader` inlined,
        // but with `glDrawElementsInstanced` and not `glDrawElements`.
        DEFER(glBindVertexArray(0)) {
            for (usize i = 0; i < rock_model.meshes_len; ++i) {
                Mesh *mesh = &rock_model.meshes[i];
                glBindVertexArray(mesh->vao);
                glDrawElementsInstanced(
                    GL_TRIANGLES, mesh->indices_len, GL_UNSIGNED_INT, 0, r->instance_count);
            }
        }
#else
        for (usize i = 0; i < r->instance_count; ++i) {
            set_shader_mat4(rock.shader, "local_to_world", r->instance_model_matrices[i]);
            draw_model_with_shader(&rock_model, &rock.shader);
        }
#endif
    }

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

#if USE_MSAA
    // Blit the multisampled framebuffer to the default one.
    DEFER(glBindFramebuffer(GL_FRAMEBUFFER, 0)) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, r->fbo_msaa);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(
            0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }
#endif
}

//
// Shaders' first run setup.
//

static inline void setup_shaders(void) {
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
        reload_shader_from_filepath(&planet.shader, planet.paths);
        reload_shader_from_filepath(&rock.shader, rock.paths);

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
