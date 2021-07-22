#include "main.h"

#define USE_MSAA 1
#define MSAA_SAMPLES 4

#define USE_INSTANCED_RENDERING 1

static Camera camera;

static vec2 mouse_last;
static bool mouse_is_first = true;
static bool is_tab_pressed = false;

static PathsToShader skybox = { {
    .vertex = GLOW_SHADERS_ "simple_skybox.vs",
    .fragment = GLOW_SHADERS_ "simple_skybox.fs",
} };
static PathsToShader planet = { {
    .vertex = GLOW_SHADERS_ "instancing_texture_test.vs",
    .fragment = GLOW_SHADERS_ "instancing_texture_test.fs",
} };
static PathsToShader rock = { {
#if USE_INSTANCED_RENDERING
    .vertex = GLOW_SHADERS_ "instancing_texture_test2.vs",
#else
    .vertex = GLOW_SHADERS_ "instancing_texture_test.vs",
#endif
    .fragment = GLOW_SHADERS_ "instancing_texture_test.fs",
} };

#define RANDOM(a, b) ((a) + ((b) - (a)) * ((f32) rand() / (f32) RAND_MAX))

int main(int argc, char *argv[]) {
    Err err = Err_None;

    WindowSettings const window_settings = { 800, 600, set_window_callbacks };
    mouse_last.x = (f32) window_settings.width / 2.0f;
    mouse_last.y = (f32) window_settings.height / 2.0f;

    camera = new_camera_at((vec3) { 0, 0, 55 });
    camera.aspect = (f32) window_settings.width / (f32) window_settings.height;
    camera.movement_speed = 50.0f;
    camera.far = 1000.0f;

    GLFWwindow *const window = init_opengl(window_settings, &err);
    /* glfwSetWindowUserPointer(GLFWwindow *window, void *pointer); */

    // @Volatile: use these same files in `process_input`.
    skybox.shader = new_shader_from_filepath(skybox.paths, &err);
    planet.shader = new_shader_from_filepath(planet.paths, &err);
    rock.shader = new_shader_from_filepath(rock.paths, &err);

    stbi_set_flip_vertically_on_load(choose_model[PLANET].flip_on_load);
    Model planet_model = alloc_new_model_from_filepath(choose_model[PLANET].path, &err);

    stbi_set_flip_vertically_on_load(choose_model[ROCK].flip_on_load);
    Model rock_model = alloc_new_model_from_filepath(choose_model[ROCK].path, &err);

    stbi_set_flip_vertically_on_load(false);
    Texture const skybox_tex = new_cubemap_texture_from_filepaths(
        (char const *[6]) {
            GLOW_TEXTURES_ "skybox/right.jpg", // +X
            GLOW_TEXTURES_ "skybox/left.jpg", // -X
            GLOW_TEXTURES_ "skybox/top.jpg", // +Y
            GLOW_TEXTURES_ "skybox/bottom.jpg", // -Y
            GLOW_TEXTURES_ "skybox/front.jpg", // +Z
            GLOW_TEXTURES_ "skybox/back.jpg", // -Z
        },
        &err);

    // Exit early if there were any errors during setup.
    if (err) { goto main_err; }

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

    // Configure the MSAA framebuffer.
    uint fbo_msaa;
    glGenFramebuffers(1, &fbo_msaa);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_msaa);
    DEFER(glBindFramebuffer(GL_FRAMEBUFFER, 0)) {
        // Create a multisampled color attachment texture.
        uint texure_ms;
        glGenTextures(1, &texure_ms);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texure_ms);
        DEFER(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0)) {
            glTexImage2DMultisample(
                /*target*/ GL_TEXTURE_2D_MULTISAMPLE,
                /*samples*/ MSAA_SAMPLES,
                /*internalformat*/ GL_RGB,
                /*width*/ window_settings.width,
                /*height*/ window_settings.height,
                /*fixedsamplelocations*/ GL_TRUE);
        }

        glFramebufferTexture2D(
            /*target*/ GL_FRAMEBUFFER,
            /*attachment*/ GL_COLOR_ATTACHMENT0,
            /*textarget*/ GL_TEXTURE_2D_MULTISAMPLE,
            /*texture*/ texure_ms,
            /*level*/ 0);

        // Create a multisampled renderbuffer object for depth and stencil attachments.
        uint rbo_ms;
        glGenRenderbuffers(1, &rbo_ms);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo_ms);
        DEFER(glBindRenderbuffer(GL_RENDERBUFFER, 0)) {
            glRenderbufferStorageMultisample(
                /*target*/ GL_RENDERBUFFER,
                /*samples*/ MSAA_SAMPLES,
                /*internalformat*/ GL_DEPTH24_STENCIL8,
                /*width*/ window_settings.width,
                /*height*/ window_settings.height);
        }

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

    f32 const offset = 2.5f; // 25.0f;
    f32 const radius = 50.0f; // 150.0f;
    usize const amount = 10000; // 100000;
    mat4 *model_matrices = calloc(amount, sizeof(mat4));
    srand((uint) glfwGetTime()); // initialize random seed
    for (usize i = 0; i < amount; ++i) {
        f32 const rotation_angle = RANDOM(0.0f, 360.0f);
        f32 const translation_angle = ((f32) i / (f32) amount) * 360.0f;

        vec3 const displacement = {
            .x = RANDOM(-offset, offset) + sinf(translation_angle) * radius,
            .y = RANDOM(-offset, offset) * 0.4f,
            .z = RANDOM(-offset, offset) + cosf(translation_angle) * radius,
        };

        mat4 const T = mat4_translate(displacement);
        mat4 const S = mat4_scale(vec3_of(RANDOM(0.05f, 0.25f)));
        mat4 const R = mat4_rotate(rotation_angle, (vec3) { 0.4f, 0.6f, 0.8f });

        model_matrices[i] = mat4_mul(T, mat4_mul(S, R));
    }

#if USE_INSTANCED_RENDERING
    // Use instanced arrays instead of passing the model matrix values as uniforms to the shader.
    // @Note: these are defined as a vertex attribute (allowing us to store much more data) that
    // are updated per instance (with `divisor` = 1) instead of per vertex (with `divisor` = 0).

    uint vbo_instance;
    glGenBuffers(1, &vbo_instance);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_instance);
    DEFER(glBindBuffer(GL_ARRAY_BUFFER, 0)) {
        glBufferData(GL_ARRAY_BUFFER, sizeof(mat4) * amount, &model_matrices[0], GL_STATIC_DRAW);
        for (usize i = 0; i < rock_model.meshes_len; ++i) {
            glBindVertexArray(rock_model.meshes[i].vao);
            DEFER(glBindVertexArray(0)) {
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

    Clock clock = { 0 };
    Fps fps = { 0 };
    setup_shaders();

    while (!glfwWindowShouldClose(window)) {
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

        mat4 const projection = get_camera_projection_matrix(&camera);
        mat4 const view = get_camera_view_matrix(&camera);

#if USE_MSAA
        // Draw the scene to the multisampled framebuffer.
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_msaa);
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
                        GL_TRIANGLES, mesh->indices_len, GL_UNSIGNED_INT, 0, amount);
                }
            }
#else
            for (usize i = 0; i < amount; ++i) {
                set_shader_mat4(rock.shader, "local_to_world", model_matrices[i]);
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

            glBindVertexArray(vao_skybox);
            bind_texture_to_unit(skybox_tex, GL_TEXTURE0);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glDepthFunc(GL_LESS);

#if USE_MSAA
        // Blit the multisampled framebuffer to the default one.
        // glBlitNamedFramebuffer(
        //     /*readFramebuffer*/ fbo_msaa,
        //     /*drawFramebuffer*/ 0,
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_msaa);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(
            0,
            0,
            window_settings.width,
            window_settings.height,
            0,
            0,
            window_settings.width,
            window_settings.height,
            /*mask*/ GL_COLOR_BUFFER_BIT,
            /*filter*/ GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao_skybox);
    glDeleteProgram(skybox.shader.program_id);
    glDeleteProgram(planet.shader.program_id);
    glDeleteProgram(rock.shader.program_id);
    dealloc_model(&planet_model);
    dealloc_model(&rock_model);
    free(model_matrices);

    goto main_exit;

main_err:
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

main_exit:
    glfwDestroyWindow(window);
    glfwTerminate();
    return err ? EXIT_FAILURE : EXIT_SUCCESS;
}

void setup_shaders(void) {
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

void process_input(GLFWwindow *window, f32 delta_time) {
    if IS_PRESSED (ESCAPE) { glfwSetWindowShouldClose(window, true); }

    if IS_PRESSED (W) { update_camera_position(&camera, CameraMovement_Forward, delta_time); }
    if IS_PRESSED (S) { update_camera_position(&camera, CameraMovement_Backward, delta_time); }
    if IS_PRESSED (A) { update_camera_position(&camera, CameraMovement_Left, delta_time); }
    if IS_PRESSED (D) { update_camera_position(&camera, CameraMovement_Right, delta_time); }
    if IS_PRESSED (E) { update_camera_position(&camera, CameraMovement_Up, delta_time); }
    if IS_PRESSED (Q) { update_camera_position(&camera, CameraMovement_Down, delta_time); }

    if ON_PRESS (TAB, is_tab_pressed) {
        GLOW_LOG("Hot swapping shaders");

        // @Volatile: use the same files as in `main`.
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

void set_window_callbacks(GLFWwindow *window) {
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
}
