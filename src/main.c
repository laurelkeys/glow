#include "main.inl"

static inline void setup_shaders(void);
static inline void process_input(GLFWwindow *window, f32 delta_time);
static void set_window_callbacks(GLFWwindow *window);

static inline Resources create_resources(Err *err, int width, int height);
static inline void destroy_resources(Resources *r, int width, int height);
static inline void begin_frame(GLFWwindow *window, int width, int height);
static inline void draw_frame(Resources const *r, int width, int height);
static inline void end_frame(GLFWwindow *window, int width, int height);

int main(int argc, char *argv[]) {
    Err err = Err_None;

    Options const options = parse_args(argc, argv);
    WindowSettings const window_settings = {
        1280, 720, set_window_callbacks, options.msaa, options.vsync, options.fullscreen,
    };

    GLFWwindow *window = init_opengl(window_settings, &err);
    if (err) { goto main_exit_opengl; }

    is_ui_enabled = !options.no_ui;
    init_imgui(window);

    int w = 0, h = 0;
    glfwGetFramebufferSize(window, &w, &h);
    assert(w > 0 && h > 0);

    mouse_last.x = 0.5f * w;
    mouse_last.y = 0.5f * h;

    camera = new_camera_at((vec3) { 0, 0, 5 });
    camera.aspect = (f32) w / (f32) h;

    Resources r = create_resources(&err, w, h);
    if (err) { goto main_exit; }

    setup_shaders();
    glEnable(GL_DEPTH_TEST);
    glfwSetInputMode(
        window, GLFW_CURSOR, mouse_is_in_ui ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    glfwSetWindowUserPointer(window, (void *) &r);

    while (!glfwWindowShouldClose(window)) {
        glfwGetFramebufferSize(window, &w, &h);
        begin_frame(window, w, h);
        draw_frame(&r, w, h);
        end_frame(window, w, h);
    }

    assert(err == Err_None);

main_exit:
    destroy_resources(&r, w, h);

    deinit_imgui();

main_exit_opengl:
    deinit_opengl(window);

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

    geometry_pass.paths.vertex = GLOW_SHADERS_ "gbuffer.vs";
    geometry_pass.paths.fragment = GLOW_SHADERS_ "gbuffer.fs";

    lighting_pass.paths.vertex = GLOW_SHADERS_ "deferred_shading.vs";
    lighting_pass.paths.fragment = GLOW_SHADERS_ "deferred_shading.fs";

    light_box.paths.vertex = GLOW_SHADERS_ "deferred_light_box.vs";
    light_box.paths.fragment = GLOW_SHADERS_ "deferred_light_box.fs";

    ssao.paths.vertex = GLOW_SHADERS_ "gbuffer.vs";
    ssao.paths.fragment = GLOW_SHADERS_ "gbuffer_ssao.fs";

    // @Volatile: use the same shaders as in `process_input`.
    geometry_pass.shader = new_shader_from_filepath(geometry_pass.paths, err);
    lighting_pass.shader = new_shader_from_filepath(lighting_pass.paths, err);
    light_box.shader = new_shader_from_filepath(light_box.paths, err);
    ssao.shader = new_shader_from_filepath(ssao.paths, err);

    stbi_set_flip_vertically_on_load(choose_model[BACKPACK].flip_on_load);
    backpack = alloc_new_model_from_filepath(choose_model[BACKPACK].path, err);

#if 0
    skybox.paths.vertex = GLOW_SHADERS_ "simple_skybox.vs";
    skybox.paths.fragment = GLOW_SHADERS_ "simple_skybox.fs";

    debug_quad.paths.vertex = GLOW_SHADERS_ "simple_quad.vs";
    debug_quad.paths.fragment = GLOW_SHADERS_ "simple_quad_depth.fs";

    test_scene.paths.vertex = GLOW_SHADERS_ "shadow_mapping.vs";
    test_scene.paths.fragment = GLOW_SHADERS_ "shadow_mapping.fs";

    shadow_mapping.paths.vertex = GLOW_SHADERS_ "shadow_mapping_depth.vs";
    shadow_mapping.paths.fragment = GLOW_SHADERS_ "shadow_mapping_depth.fs";

    skybox.shader = new_shader_from_filepath(skybox.paths, err);
    test_scene.shader = new_shader_from_filepath(test_scene.paths, err);
    debug_quad.shader = new_shader_from_filepath(debug_quad.paths, err);
    shadow_mapping.shader = new_shader_from_filepath(shadow_mapping.paths, err);

    skybox_texture = new_cubemap_texture_from_filepaths(
        (char const *[6]) {
            GLOW_TEXTURES_ "skybox/right.jpg", // +X
            GLOW_TEXTURES_ "skybox/left.jpg", // -X
            GLOW_TEXTURES_ "skybox/top.jpg", // +Y
            GLOW_TEXTURES_ "skybox/bottom.jpg", // -Y
            GLOW_TEXTURES_ "skybox/front.jpg", // +Z
            GLOW_TEXTURES_ "skybox/back.jpg", // -Z
        },
        (TextureSettings) { .flip_vertically = false },
        err);

    wood_texture = new_texture_from_filepath(
        GLOW_TEXTURES_ "wood.png",
        (TextureSettings) { .flip_vertically = true, .generate_mipmap = true },
        err);
#endif

    // Exit early if there were any errors during setup.
    if (*err != Err_None) { return r; }

    //
    // Scene's objects and lights (object_positions, light_positions, light_colors).
    //

    STATIC_ASSERT(OBJECT_COUNT == 9);
    r.object_positions[0] = (vec3) { -3, -0.5, -3 };
    r.object_positions[1] = (vec3) { +0, -0.5, -3 };
    r.object_positions[2] = (vec3) { +3, -0.5, -3 };
    r.object_positions[3] = (vec3) { -3, -0.5, +0 };
    r.object_positions[4] = (vec3) { +0, -0.5, +0 };
    r.object_positions[5] = (vec3) { +3, -0.5, +0 };
    r.object_positions[6] = (vec3) { -3, -0.5, +3 };
    r.object_positions[7] = (vec3) { +0, -0.5, +3 };
    r.object_positions[8] = (vec3) { +3, -0.5, +3 };

    for (usize i = 0; i < LIGHT_COUNT; ++i) {
        r.light_positions[i] = (vec3) {
            ((rand() % 100) / 100.0) * 6.0 - 3.0,
            ((rand() % 100) / 100.0) * 6.0 - 4.0,
            ((rand() % 100) / 100.0) * 6.0 - 3.0,
        };
        r.light_colors[i] = (vec3) {
            ((rand() % 100) / 200.0) + 0.5,
            ((rand() % 100) / 200.0) + 0.5,
            ((rand() % 100) / 200.0) + 0.5,
        };
    }

    //
    // SSAO - Screen-space ambient occlusion (ssao_sample_kernel, ssao_noise,
    // tex_noise, fbo_ssao, tex_ssao, fbo_ssao_blur, tex_ssao_blur).
    //

    for (usize i = 0; i < ARRAY_LEN(ssao_sample_kernel); ++i) {
        // @Note: vary z only in [0.0, 1.0] in tangent space so that we sample from a hemisphere.
        ssao_sample_kernel[i] = vec3_scl(
            vec3_normalize((vec3) { RANDOM(-1, 1), RANDOM(-1, 1), RANDOM(0, 1) }), RANDOM(0, 1));
        // @Note: by normalizing the vector we push it to the hemisphere's surface. So, to sample
        // within it, we multiply by a random value and then `scale` biases it towards the center.
        f32 const scale = (f32) i / (f32) ARRAY_LEN(ssao_sample_kernel);
        ssao_sample_kernel[i] = vec3_scl(ssao_sample_kernel[i], lerp(0.1f, 1.0f, scale * scale));
    }

    for (usize i = 0; i < ARRAY_LEN(ssao_noise); ++i) {
        ssao_noise[i] = (vec3) { .x = RANDOM(-1, 1), .y = RANDOM(-1, 1), .z = 0 };
    }

    glGenTextures(1, &r.tex_noise);
    glBindTexture(GL_TEXTURE_2D, r.tex_noise);
    STATIC_ASSERT(ARRAY_LEN(ssao_noise) == 4 * 4);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssao_noise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    /* clang-format off */

    #define SSAO_COLOR_BUFFER(gl_handle)                                                  \
        glGenTextures(1, &gl_handle);                                                     \
        glBindTexture(GL_TEXTURE_2D, gl_handle);                                          \
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_FLOAT, NULL); \
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);                \
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);                \
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gl_handle, 0);

    glGenFramebuffers(1, &r.fbo_ssao);
    glBindFramebuffer(GL_FRAMEBUFFER, r.fbo_ssao);
    DEFER(glBindFramebuffer(GL_FRAMEBUFFER, 0)) {
        SSAO_COLOR_BUFFER(r.tex_ssao);
        // check_bound_framebuffer_is_complete();
    }

    glGenFramebuffers(1, &r.fbo_ssao_blur);
    glBindFramebuffer(GL_FRAMEBUFFER, r.fbo_ssao_blur);
    DEFER(glBindFramebuffer(GL_FRAMEBUFFER, 0)) {
        SSAO_COLOR_BUFFER(r.tex_ssao_blur);
        // check_bound_framebuffer_is_complete();
    }

    #undef SSAO_COLOR_BUFFER

    /* clang-format on */

    //
    // Configure the g-buffer (gbuffer, gtex_position, gtex_normal, gtex_albedo_spec,
    // grbo_depth).
    //

    glGenFramebuffers(1, &r.gbuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, r.gbuffer);
    DEFER(glBindFramebuffer(GL_FRAMEBUFFER, 0)) {
        /* clang-format off */

        #define COLOR_BUFFER(                                                                               \
                gl_handle, gl_internal_format, gl_format, gl_type, gl_color_attachment, gl_texture_wrap)    \
            glGenTextures(1, &gl_handle);                                                                   \
            glBindTexture(GL_TEXTURE_2D, gl_handle);                                                        \
            glTexImage2D(GL_TEXTURE_2D, 0, gl_internal_format, width, height, 0, gl_format, gl_type, NULL); \
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);                              \
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);                              \
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, gl_texture_wrap);                             \
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, gl_texture_wrap);                             \
            glFramebufferTexture2D(GL_FRAMEBUFFER, gl_color_attachment, GL_TEXTURE_2D, gl_handle, 0);

        // Position color buffer.
        COLOR_BUFFER(r.gtex_position, GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_COLOR_ATTACHMENT0, GL_CLAMP_TO_EDGE);

        // Normal color buffer.
        COLOR_BUFFER(r.gtex_normal, GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_COLOR_ATTACHMENT1, GL_REPEAT);

        // Albedo color + specular intensity color buffer.
        COLOR_BUFFER(r.gtex_albedo_spec, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GL_COLOR_ATTACHMENT2, GL_REPEAT);

        // Specify which color attachments will be used for rendering.
        glDrawBuffers(3, (uint[3]) { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 });

        // Depth buffer renderbuffer.
        glGenRenderbuffers(1, &r.grbo_depth);
        glBindRenderbuffer(GL_RENDERBUFFER, r.grbo_depth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, r.grbo_depth);

        check_bound_framebuffer_is_complete();

        #undef GBUFFER_COLOR_BUFFER

        /* clang-format on */
    }

#if 0
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
    // Scene description (light_position, vao_plane, vao_cube).
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
                    2, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 8, (void *) (sizeof(f32) * 6));
            }
        }
    }

    glGenVertexArrays(1, &r.vao_cube);
    {
        uint vbo;
        glGenBuffers(1, &vbo);
        DEFER(glDeleteBuffers(1, &vbo)) {
            glBindVertexArray(r.vao_cube);
            DEFER(glBindVertexArray(0)) {
                f32 cube_vertices_ndc[ARRAY_LEN(CUBE_VERTICES)];
                memcpy(cube_vertices_ndc, CUBE_VERTICES, sizeof(CUBE_VERTICES));
                for (usize i = 0; i < ARRAY_LEN(CUBE_VERTICES); i += 8) {
                    // Map [-0.5, 0.5] to [-1.0. 1.0].
                    cube_vertices_ndc[i + 0] *= 2.0f;
                    cube_vertices_ndc[i + 1] *= 2.0f;
                    cube_vertices_ndc[i + 2] *= 2.0f;
                }

                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                glBufferData(
                    GL_ARRAY_BUFFER,
                    sizeof(cube_vertices_ndc),
                    cube_vertices_ndc,
                    GL_STATIC_DRAW);

                glEnableVertexAttribArray(0); // position
                glEnableVertexAttribArray(1); // normal
                glEnableVertexAttribArray(2); // texcoord

                glVertexAttribPointer(
                    0, 3, GL_FLOAT, GL_FALSE, sizeof(f32) * 8, (void *) (sizeof(f32) * 0));
                glVertexAttribPointer(
                    1, 3, GL_FLOAT, GL_FALSE, sizeof(f32) * 8, (void *) (sizeof(f32) * 3));
                glVertexAttribPointer(
                    2, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 8, (void *) (sizeof(f32) * 6));
            }
        }
    }

    //
    // Shadow mapping (vao_debug_quad, tex_depth_map, fbo_depth_map).
    //

    glGenVertexArrays(1, &r.vao_debug_quad);
    {
        uint vbo;
        glGenBuffers(1, &vbo);
        DEFER(glDeleteBuffers(1, &vbo)) {
            glBindVertexArray(r.vao_debug_quad);
            DEFER(glBindVertexArray(0)) {
                // @Note: use GL_TRIANGLE_STRIP to draw it.
                f32 const quad_vertices_ndc[] = {
                    -1, 1, 0, 1, -1, -1, 0, 0, 1, 1, 1, 1, 1, -1, 1, 0,
                };

                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                glBufferData(
                    GL_ARRAY_BUFFER,
                    sizeof(quad_vertices_ndc),
                    quad_vertices_ndc,
                    GL_STATIC_DRAW);

                glEnableVertexAttribArray(0); // position (2D)
                glEnableVertexAttribArray(1); // texcoord

                glVertexAttribPointer(
                    0, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 4, (void *) (sizeof(f32) * 0));
                glVertexAttribPointer(
                    1, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 4, (void *) (sizeof(f32) * 2));
            }
        }
    }

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
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        // @Note: set coordinates outside the depth map's range to have a depth of 1.0.
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, (f32[]) { 1, 1, 1, 1 });
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

        check_bound_framebuffer_is_complete();
    }
#endif

    assert(*err == Err_None);
    return r;
}

static inline void destroy_resources(Resources *r, int width, int height) {
    UNUSED(width);
    UNUSED(height);

    glDeleteTextures(1, &r->tex_noise);
    glDeleteTextures(1, &r->tex_ssao);
    glDeleteFramebuffers(1, &r->fbo_ssao);

    glDeleteRenderbuffers(1, &r->grbo_depth);
    glDeleteTextures(1, &r->gtex_albedo_spec);
    glDeleteTextures(1, &r->gtex_normal);
    glDeleteTextures(1, &r->gtex_position);
    glDeleteFramebuffers(1, &r->gbuffer);

#if 0
    glDeleteFramebuffers(1, &r->fbo_depth_map);
    glDeleteTextures(1, &r->tex_depth_map);
    glDeleteVertexArrays(1, &r->vao_debug_quad);
    glDeleteVertexArrays(1, &r->vao_cube);
    glDeleteVertexArrays(1, &r->vao_plane);
    glDeleteVertexArrays(1, &r->vao_skybox);
    glDeleteFramebuffers(1, &r->gbuffer);

    glDeleteTextures(1, &wood_texture.id);
    glDeleteTextures(1, &skybox_texture.id);
#endif

    dealloc_model(&backpack);

#if 0
    glDeleteProgram(shadow_mapping.shader.program_id);
    glDeleteProgram(debug_quad.shader.program_id);
    glDeleteProgram(test_scene.shader.program_id);
    glDeleteProgram(skybox.shader.program_id);
#endif

    glDeleteProgram(ssao.shader.program_id);
    glDeleteProgram(light_box.shader.program_id);
    glDeleteProgram(lighting_pass.shader.program_id);
    glDeleteProgram(geometry_pass.shader.program_id);
}

//
// Frame rendering pre- and post-processing.
//

static inline void begin_frame(GLFWwindow *window, int width, int height) {
    UNUSED(width);
    UNUSED(height);

    clock_tick(&clock, glfwGetTime());
    update_frame_counter(&frame_counter, clock.time);
    process_input(window, clock.time_increment);

    if (frame_counter.last_update_time == clock.time) {
        char title[64]; // 64 seems large enough..
        snprintf(
            title,
            sizeof(title),
            "glow | %.2f mspf | %d fps",
            frame_counter.frame_interval,
            (int) round(1000.0 / frame_counter.frame_interval));
        glfwSetWindowTitle(window, title);
    }

    if (is_ui_enabled) { begin_imgui_frame(); }
}

static inline void end_frame(GLFWwindow *window, int width, int height) {
    UNUSED(width);
    UNUSED(height);

    if (is_ui_enabled) { end_imgui_frame(); }

    glfwSwapBuffers(window);
    glfwPollEvents();
}

//
// Frame rendering.
//

#define NEAR_PLANE 1.0f
#define FAR_PLANE 7.5f

#if 0
static inline void render_scene_with_shader(Shader const shader, Resources const *r) {
    DEFER(glBindVertexArray(0)) {
        mat4 model = mat4_id();
        set_shader_mat4(shader, "local_to_world", model);
        glBindVertexArray(r->vao_plane);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        model = mat4_mul(mat4_translate((vec3) { 0.0f, 1.5f, 0.0 }), mat4_scale(vec3_of(0.5f)));
        set_shader_mat4(shader, "local_to_world", model);
        glBindVertexArray(r->vao_cube);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        model = mat4_mul(mat4_translate((vec3) { 2.0f, 0.0f, 1.0 }), mat4_scale(vec3_of(0.5f)));
        set_shader_mat4(shader, "local_to_world", model);
        glBindVertexArray(r->vao_cube);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        model = mat4_mul(
            mat4_translate((vec3) { -1.0f, 0.0f, 2.0 }),
            mat4_mul(mat4_rotate(RADIANS(60), (vec3) { 1, 0, 1 }), mat4_scale(vec3_of(0.25))));
        set_shader_mat4(shader, "local_to_world", model);
        glBindVertexArray(r->vao_cube);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}
#endif

static inline void render_quad(void) {
    static uint vao_quad = 0;

    /* clang-format off */
    if (vao_quad == 0) {
        glGenVertexArrays(1, &vao_quad); // @Leak

        uint vbo;
        glGenBuffers(1, &vbo);
        DEFER(glDeleteBuffers(1, &vbo)) {
            glBindVertexArray(vao_quad);
            DEFER(glBindVertexArray(0)) {
                f32 const quad_vertices_ndc[] = { -1, 1, 0, 0, 1, -1, -1, 0, 0, 0, 1, 1, 0, 1, 1, 1, -1, 0, 1, 0 };
                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices_ndc), quad_vertices_ndc, GL_STATIC_DRAW);

                glEnableVertexAttribArray(0); // position
                glEnableVertexAttribArray(1); // texcoord

                glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 5, (void *) (sizeof(f32) * 0));
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 5, (void *) (sizeof(f32) * 3));
            }
        }
    }
    /* clang-format on */

    DEFER(glBindVertexArray(0)) {
        glBindVertexArray(vao_quad);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
}

static inline void render_cube(void) {
    static uint vao_cube = 0;

    /* clang-format off */
    if (vao_cube == 0) {
        glGenVertexArrays(1, &vao_cube); // @Leak

        uint vbo;
        glGenBuffers(1, &vbo);
        DEFER(glDeleteBuffers(1, &vbo)) {
            glBindVertexArray(vao_cube);
            DEFER(glBindVertexArray(0)) {
                static f32 const cube_vertices_ndc[] = { -1, -1, -1, 0, 0, -1, 0, 0, 1, 1, -1, 0, 0, -1, 1, 1, 1, -1, -1, 0, 0, -1, 1, 0, 1, 1, -1, 0, 0, -1, 1, 1, -1, -1, -1, 0, 0, -1, 0, 0, -1, 1, -1, 0, 0, -1, 0, 1, -1, -1, 1, 0, 0, 1, 0, 0, 1, -1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, -1, 1, 1, 0, 0, 1, 0, 1, -1, -1, 1, 0, 0, 1, 0, 0, -1, 1, 1, -1, 0, 0, 1, 0, -1, 1, -1, -1, 0, 0, 1, 1, -1, -1, -1, -1, 0, 0, 0, 1, -1, -1, -1, -1, 0, 0, 0, 1, -1, -1, 1, -1, 0, 0, 0, 0, -1, 1, 1, -1, 0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0, 1, -1, -1, 1, 0, 0, 0, 1, 1, 1, -1, 1, 0, 0, 1, 1, 1, -1, -1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 1, -1, 1, 1, 0, 0, 0, 0, -1, -1, -1, 0, -1, 0, 0, 1, 1, -1, -1, 0, -1, 0, 1, 1, 1, -1, 1, 0, -1, 0, 1, 0, 1, -1, 1, 0, -1, 0, 1, 0, -1, -1, 1, 0, -1, 0, 0, 0, -1, -1, -1, 0, -1, 0, 0, 1, -1, 1, -1, 0, 1, 0, 0, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, -1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, -1, 1, -1, 0, 1, 0, 0, 1, -1, 1, 1, 0, 1, 0, 0, 0 };
                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices_ndc), cube_vertices_ndc, GL_STATIC_DRAW);

                glEnableVertexAttribArray(0); // position
                glEnableVertexAttribArray(1); // normal
                glEnableVertexAttribArray(2); // texcoord

                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(f32) * 8, (void *) (sizeof(f32) * 0));
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(f32) * 8, (void *) (sizeof(f32) * 3));
                glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 8, (void *) (sizeof(f32) * 6));
            }
        }
    }
    /* clang-format on */

    DEFER(glBindVertexArray(0)) {
        glBindVertexArray(vao_cube);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}

static inline void draw_frame(Resources const *r, int width, int height) {
    mat4 const projection = compute_camera_projection_matrix(&camera);
    mat4 const view = compute_camera_view_matrix(&camera);

    // @Note: clear to black to avoid leaking into the g-buffer.
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //
    // Geometry pass (render all geometric and color data to the g-buffer).
    //

    DEFER(glBindFramebuffer(GL_FRAMEBUFFER, 0)) {
        glBindFramebuffer(GL_FRAMEBUFFER, r->gbuffer);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        use_shader(geometry_pass.shader);
        {
            set_shader_mat4(geometry_pass.shader, "world_to_view", view);
            set_shader_mat4(geometry_pass.shader, "view_to_clip", projection);

            for (usize i = 0; i < OBJECT_COUNT; ++i) {
                set_shader_mat4(
                    geometry_pass.shader,
                    "local_to_world",
                    mat4_mul(mat4_translate(r->object_positions[i]), mat4_scale(vec3_of(0.5f))));

                draw_model_with_shader(&backpack, &geometry_pass.shader);
            }
        }
    }

    //
    // Use the g-buffer to render SSAO texture.
    //

    DEFER(glBindFramebuffer(GL_FRAMEBUFFER, 0)) {
        glBindFramebuffer(GL_FRAMEBUFFER, r->fbo_ssao);

        glClear(GL_COLOR_BUFFER_BIT);
        use_shader(ssao.shader);
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, r->gtex_position);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, r->gtex_normal);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, r->tex_noise);

            // @Todo: send kernel samples to shader.

            set_shader_mat4(ssao.shader, "view_to_clip", projection);

            render_quad();
        }
    }

    //
    // Deferred lighting pass (use g-buffer to calculate scene's lighting).
    //

    static int draw_mode = DRAW_LIGHTING;
    imgui_slider_int("draw_mode", &draw_mode, 0, 5);

    static int dark_threshold = 5;
    imgui_slider_int("dark_threshold", &dark_threshold, 1, 255);

    static float constant = 1.0f;
    static float linear = 0.7f;
    static float quadratic = 1.8f;
    imgui_slider_float("constant", &constant, 0.0f, 10.0f);
    imgui_slider_float("linear", &linear, 0.0f, 10.0f);
    imgui_slider_float("quadratic", &quadratic, 0.0f, 10.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    use_shader(lighting_pass.shader);
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, r->gtex_position);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, r->gtex_normal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, r->gtex_albedo_spec);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, r->tex_ssao);

        char uniform_string[32]; // 32 seems large enough..
        for (usize i = 0; i < LIGHT_COUNT; ++i) {
            vec3 const color = r->light_colors[i];

            snprintf(uniform_string, 32, "lights[%zu].position", i);
            set_shader_vec3(lighting_pass.shader, uniform_string, r->light_positions[i]);

            snprintf(uniform_string, 32, "lights[%zu].color", i);
            set_shader_vec3(lighting_pass.shader, uniform_string, color);

            // Threshold = I_max / (Kc + Kl * d + Kq * d*d)
            // Kq * d*d + Kl * d + Kc - (I_max / Threshold) = 0
            f32 const max_intensity = MAX3(color.x, color.y, color.z);
            f32 const a = quadratic;
            f32 const b = linear;
            f32 const c = constant - (max_intensity * (256.0f / dark_threshold));
            f32 const effect_radius =
                (a == 0.0f) ? -c / b : (-b + sqrtf(b * b - 4 * a * c)) / (2 * a);

            snprintf(uniform_string, 32, "lights[%zu].radius", i);
            set_shader_float(lighting_pass.shader, uniform_string, effect_radius);

            snprintf(uniform_string, 32, "lights[%zu].constant", i);
            set_shader_float(lighting_pass.shader, uniform_string, constant);

            snprintf(uniform_string, 32, "lights[%zu].linear", i);
            set_shader_float(lighting_pass.shader, uniform_string, linear);

            snprintf(uniform_string, 32, "lights[%zu].quadratic", i);
            set_shader_float(lighting_pass.shader, uniform_string, quadratic);
        }

        set_shader_vec3(lighting_pass.shader, "view_pos", camera.position);

        set_shader_int(lighting_pass.shader, "draw_mode", draw_mode); // @@

        render_quad();
    }

    //
    // Forward rendering pass (to render all light cubes).
    //

    DEFER(glBindFramebuffer(GL_FRAMEBUFFER, 0)) {
        // @Note: copy the depth values from the g-buffer into the default framebuffer,
        // this way the lights don't end up getting rendered on top of everything else.
        glBindFramebuffer(GL_READ_FRAMEBUFFER, r->gbuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(
            0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    }

    use_shader(light_box.shader);
    {
        set_shader_mat4(light_box.shader, "world_to_view", view);
        set_shader_mat4(light_box.shader, "view_to_clip", projection);

        for (usize i = 0; i < LIGHT_COUNT; ++i) {
            set_shader_mat4(
                light_box.shader,
                "local_to_world",
                mat4_mul(mat4_translate(r->light_positions[i]), mat4_scale(vec3_of(0.125f))));

            set_shader_vec3(light_box.shader, "light_color", r->light_colors[i]);

            render_cube();
        }
    }

#if !1
    // Use an orthographic projection matrix to model a directional light source
    // (i.e. all its rays are parallel), so there is no perspective deform.
    mat4 const light_projection = mat4_ortho(-10, 10, -10, 10, NEAR_PLANE, FAR_PLANE);
    mat4 const light_view =
        mat4_lookat(r->light_position, /*target*/ (vec3) { 0 }, /*up*/ (vec3) { 0, 1, 0 });
    mat4 const light_space_matrix = mat4_mul(light_projection, light_view);

    // @Note: first render depth values to a texture, from the light's perspective,
    // then render the scene as normal with shadow mapping (by using the depth map).
    glViewport(0, 0, SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION);
    DEFER(glViewport(0, 0, width, height)) {
        glBindFramebuffer(GL_FRAMEBUFFER, r->fbo_depth_map);
        DEFER(glBindFramebuffer(GL_FRAMEBUFFER, 0)) {
            glClear(GL_DEPTH_BUFFER_BIT);

            use_shader(shadow_mapping.shader);
            {
                set_shader_mat4(
                    shadow_mapping.shader, "world_to_light_space", light_space_matrix);

                bind_texture_to_unit(wood_texture, GL_TEXTURE0);

                render_scene_with_shader(shadow_mapping.shader, r);
            }
        }
    }

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (show_debug_quad) {
        // Render the depth map to a fullscreen quad for visual debugging.
        use_shader(debug_quad.shader);
        {
            set_shader_float(debug_quad.shader, "near_plane", NEAR_PLANE);
            set_shader_float(debug_quad.shader, "far_plane", FAR_PLANE);

            glBindVertexArray(r->vao_debug_quad);
            DEFER(glBindVertexArray(0)) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, r->tex_depth_map);

                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            }
        }
        return;
    }

    // Render the scene, but now using the generated depth/shadow map.
    use_shader(test_scene.shader);
    {
        set_shader_mat4(test_scene.shader, "world_to_view", view);
        set_shader_mat4(test_scene.shader, "view_to_clip", projection);

        set_shader_vec3(test_scene.shader, "view_pos", camera.position);
        set_shader_vec3(test_scene.shader, "light_pos", r->light_position);
        set_shader_mat4(test_scene.shader, "world_to_light_space", light_space_matrix);

        bind_texture_to_unit(wood_texture, GL_TEXTURE0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, r->tex_depth_map);

        render_scene_with_shader(test_scene.shader, r);
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
#endif
}

//
// Shaders' first run setup.
//

static inline void setup_shaders(void) {
    use_shader(lighting_pass.shader);
    set_shader_sampler2D(lighting_pass.shader, "gPosition", GL_TEXTURE0);
    set_shader_sampler2D(lighting_pass.shader, "gNormal", GL_TEXTURE1);
    set_shader_sampler2D(lighting_pass.shader, "gAlbedoSpec", GL_TEXTURE2);

#if 0
    use_shader(test_scene.shader);
    set_shader_sampler2D(test_scene.shader, "texture_diffuse", GL_TEXTURE0);
    set_shader_sampler2D(test_scene.shader, "shadow_map", GL_TEXTURE1);

    use_shader(debug_quad.shader);
    set_shader_sampler2D(debug_quad.shader, "depth_map", GL_TEXTURE0);

    use_shader(skybox.shader);
    set_shader_sampler2D(skybox.shader, "skybox", GL_TEXTURE0);
#endif
}

//
// Input processing.
//

/* clang-format off */
#define IS_PRESSED(key) (glfwGetKey(window, GLFW_KEY_##key) == GLFW_PRESS)
#define IS_RELEASED(key) (glfwGetKey(window, GLFW_KEY_##key) == GLFW_RELEASE)

// LEFT, RIGHT, MIDDLE, 1, 2, 3, 4, 5, 6, 7, 8
#define IS_PRESSED_MOUSE(btn) (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_##btn) == GLFW_PRESS)
#define IS_RELEASED_MOUSE(btn) (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_##btn) == GLFW_RELEASE)
/* clang-format on */

static inline void process_input(GLFWwindow *window, f32 delta_time) {
    if (IS_PRESSED(ESCAPE)) { glfwSetWindowShouldClose(window, true); }

    bool const is_pressed_shift = IS_PRESSED(LEFT_SHIFT) || IS_PRESSED(RIGHT_SHIFT);

    if (!mouse_is_in_ui) {
        f32 const dt = is_pressed_shift ? 10.0f * delta_time : delta_time;
        if (IS_PRESSED(W)) { update_camera_position(&camera, CameraMovement_Forward, dt); }
        if (IS_PRESSED(S)) { update_camera_position(&camera, CameraMovement_Backward, dt); }
        if (IS_PRESSED(A)) { update_camera_position(&camera, CameraMovement_Left, dt); }
        if (IS_PRESSED(D)) { update_camera_position(&camera, CameraMovement_Right, dt); }
        if (IS_PRESSED(E)) { update_camera_position(&camera, CameraMovement_Up, dt); }
        if (IS_PRESSED(Q)) { update_camera_position(&camera, CameraMovement_Down, dt); }
    }

    // Reload shader sources.
    if (IS_PRESSED(TAB) && !was_tab_pressed) {
        GLOW_LOG("Hot swapping shaders");

        // @Volatile: use the same shaders as in `create_resources`.
        try_reload_shader_from_filepath(&geometry_pass.shader, geometry_pass.paths);
        try_reload_shader_from_filepath(&lighting_pass.shader, lighting_pass.paths);
        try_reload_shader_from_filepath(&light_box.shader, light_box.paths);
#if 0
        try_reload_shader_from_filepath(&skybox.shader, skybox.paths);
        try_reload_shader_from_filepath(&test_scene.shader, test_scene.paths);
        try_reload_shader_from_filepath(&debug_quad.shader, debug_quad.paths);
        try_reload_shader_from_filepath(&shadow_mapping.shader, shadow_mapping.paths);
#endif

        setup_shaders();
    }

    // Get or release GUI control of the mouse.
    if (IS_PRESSED(SPACE) && !was_space_pressed) {
        if (mouse_is_in_ui) {
            mouse_is_in_ui = false;
            if (is_ui_enabled) { imgui_config_mouse(false); }
            glfwSetCursorPos(window, mouse_last.x, mouse_last.y);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else {
            mouse_is_in_ui = true;
            if (is_ui_enabled) { imgui_config_mouse(true); }
            glfwSetCursorPos(window, 0, 0);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }

    // @Temporary: used to see the shadow map depth texure.
    show_debug_quad = IS_PRESSED(LEFT_SHIFT) || IS_PRESSED(RIGHT_SHIFT);

    // Update key state flags.
    was_rmb_pressed = IS_PRESSED_MOUSE(RIGHT);
    was_lmb_pressed = IS_PRESSED_MOUSE(LEFT);
    was_tab_pressed = IS_PRESSED(TAB);
    was_space_pressed = IS_PRESSED(SPACE);
}

//
// Window callbacks.
//

static void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    assert(width > 0 && height > 0); // @Fixme: minimizing window
    glViewport(0, 0, width, height);

    // Update the camera's aspect ratio.
    camera.aspect = (f32) width / (f32) height;

    Resources *r = glfwGetWindowUserPointer(window);

    // Resize buffers.
    DEFER(glBindTexture(GL_TEXTURE_2D, 0)) {
        /* clang-format off */
        glBindTexture(GL_TEXTURE_2D, r->gtex_position);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);

        glBindTexture(GL_TEXTURE_2D, r->gtex_normal);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);

        glBindTexture(GL_TEXTURE_2D, r->gtex_albedo_spec);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        /* clang-format on */
    }

    glBindRenderbuffer(GL_RENDERBUFFER, r->grbo_depth);
    DEFER(glBindRenderbuffer(GL_RENDERBUFFER, 0)) {
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    }
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

    if (!mouse_is_in_ui) {
        // Reverse y since 0, 0 is the top left.
        f32 const xoffset = xpos - mouse_last.x;
        f32 const yoffset = mouse_last.y - ypos;
        update_camera_angles(&camera, (CameraMouseEvent) { xoffset, yoffset });

        mouse_last.x = xpos;
        mouse_last.y = ypos;
    }
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
