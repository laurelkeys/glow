#include "main.h"

Camera camera;

vec2 mouse_last;
bool mouse_is_first = true;
bool is_tab_pressed = false;

PathsToShader skybox = { {
    .vertex = GLOW_SHADERS_ "simple_skybox.vs",
    .fragment = GLOW_SHADERS_ "simple_skybox.fs",
} };
PathsToShader backpack = { {
    .vertex = GLOW_SHADERS_ "simple_texture.vs",
    .fragment = GLOW_SHADERS_ "simple_texture.fs",
} };
PathsToShader vis_normals = { {
    .vertex = GLOW_SHADERS_ "visualize_normals.vs",
    .fragment = GLOW_SHADERS_ "visualize_normals.fs",
    .geometry = GLOW_SHADERS_ "visualize_normals.gs",
} };

int main(int argc, char *argv[]) {
    Err err = Err_None;

    WindowSettings const window_settings = { 1280, 720, set_window_callbacks };
    mouse_last.x = (f32) window_settings.width / 2.0f;
    mouse_last.y = (f32) window_settings.height / 2.0f;

    camera = new_camera_at((vec3) { 0, 0, 3 });
    camera.aspect = (f32) window_settings.width / (f32) window_settings.height;

    GLFWwindow *const window = init_opengl(window_settings, &err);

    /* glfwSetWindowUserPointer(GLFWwindow *window, void *pointer); */

    // @Volatile: use these same files in `process_input`.
    skybox.shader = new_shader_from_filepath(skybox.paths, &err);
    backpack.shader = new_shader_from_filepath(backpack.paths, &err);
    vis_normals.shader = new_shader_from_filepath(vis_normals.paths, &err);

    stbi_set_flip_vertically_on_load(choose_model[BACKPACK].flip_on_load);
    Model backpack_model = alloc_new_model_from_filepath(choose_model[BACKPACK].path, &err);

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

#define MSAA 1
#define MSAA_SAMPLES 16

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

    Clock clock = { 0 };
    Fps fps = { 0 };
    setup_shaders();

    while (!glfwWindowShouldClose(window)) {
        clock_tick(&clock, glfwGetTime());
        update_frame_counter(&fps, clock.time);
        process_input(window, clock.time_increment);

        if (fps.last_update_time == clock.time) {
            char title[32];
            snprintf(title, sizeof(title), "glow | %d fps", fps.rate);
            glfwSetWindowTitle(window, title);
        }

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mat4 const projection = get_camera_projection_matrix(&camera);
        mat4 const view = get_camera_view_matrix(&camera);

#if MSAA
        // Draw the scene to the multisampled framebuffer.
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_msaa);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
#endif

        use_shader(backpack.shader);
        {
            set_shader_mat4(backpack.shader, "local_to_world", mat4_id());
            set_shader_mat4(backpack.shader, "world_to_view", view);
            set_shader_mat4(backpack.shader, "view_to_clip", projection);

            draw_model_with_shader(&backpack_model, &backpack.shader);
        }

        // Draw the same model, but now with a geometry shader to visualize normals.
        use_shader(vis_normals.shader);
        {
            set_shader_mat4(vis_normals.shader, "local_to_world", mat4_id());
            set_shader_mat4(vis_normals.shader, "world_to_view", view);
            set_shader_mat4(vis_normals.shader, "view_to_clip", projection);

            draw_textureless_model_with_shader(&backpack_model, &vis_normals.shader);
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

#if MSAA
        // Blit the multisampled framebuffer to the default one.
        // glBlitNamedFramebuffer(
        //     /*readFramebuffer*/ fbo_msaa,
        //     /*drawFramebuffer*/ 0,
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_msaa);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(
            /*src*/ 0, 0, window_settings.width, window_settings.height,
            /*dst*/ 0, 0, window_settings.width, window_settings.height,
            /*mask*/ GL_COLOR_BUFFER_BIT, /*filter*/ GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao_skybox);
    glDeleteProgram(skybox.shader.program_id);
    glDeleteProgram(backpack.shader.program_id);
    dealloc_model(&backpack_model);

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
        // @Volatile: use the same files as in `main`.
        GLOW_LOG("Hot swapping skybox shaders");
        reload_shader_from_filepath(&skybox.shader, skybox.paths);

        GLOW_LOG("Hot swapping backpack shaders");
        reload_shader_from_filepath(&backpack.shader, backpack.paths);

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
