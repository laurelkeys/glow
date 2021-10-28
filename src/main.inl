#include "prelude.h"

//
// Header includes.
//

#include "camera.h"
#include "console.h"
#include "file.h"
#include "imgui_facade.h"
#include "maths.h"
#include "mesh.h"
#include "model.h"
#include "opengl.h"
#include "options.h"
#include "shader.h"
#include "texture.h"
#include "vertices.h"
#include "window.h"

// Standard headers.
#include <stdio.h>
#include <string.h>

// External headers.
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <stb_image.h>

//
// Resource path macros.
//

#ifndef GLOW_SHADERS_
#define GLOW_SHADERS_ ""
#endif

#ifndef GLOW_TEXTURES_
#define GLOW_TEXTURES_ ""
#endif

#ifndef GLOW_MODELS_
#define GLOW_MODELS_ ""
#endif

//
// Helper structs/enums.
//

typedef struct PathsToShader {
    ShaderFilepaths paths;
    Shader shader;
} PathsToShader;

typedef struct PathToModel {
    char const *path;
    bool flip_on_load;
} PathToModel;

enum { BACKPACK = 0, NANOSUIT, CYBORG, PLANET, ROCK };

static PathToModel const choose_model[] = {
    [BACKPACK] = { GLOW_MODELS_ "obj/backpack/backpack.obj", true },
    [NANOSUIT] = { GLOW_MODELS_ "obj/nanosuit/nanosuit.obj", false },
    [CYBORG] = { GLOW_MODELS_ "obj/cyborg/cyborg.obj", false },
    [PLANET] = { GLOW_MODELS_ "obj/planet/planet.obj", false },
    [ROCK] = { GLOW_MODELS_ "obj/rock/rock.obj", true },
};

enum {
    DRAW_LIGHTING = 0,
    DRAW_POSITION,
    DRAW_NORMAL,
    DRAW_ALBEDO,
    DRAW_SPECULAR,
    DRAW_LIGHT_VOLUMES
};

//
// Global context properties.
//

// @Cleanup: group all of this together into some Context
// struct, with some Mouse and Keyboard groupings as well.
static bool is_ui_enabled = true;
static bool show_debug_quad = false;
static bool was_rmb_pressed = false;
static bool was_lmb_pressed = false;
static bool was_tab_pressed = false;
static bool was_space_pressed = false;
static bool mouse_is_in_ui = false;
static bool mouse_is_first = true;
static vec2 mouse_last = { 0 };
static Clock clock = { 0 };
static FrameCounter frame_counter = { 0 };

static Camera camera;

static Model backpack;

static Texture skybox_texture;
static Texture wood_texture;

static PathsToShader geometry_pass;
static PathsToShader lighting_pass;
static PathsToShader light_box;
static PathsToShader ssao;
#if 0
static PathsToShader skybox;
static PathsToShader debug_quad;
static PathsToShader test_scene;
static PathsToShader shadow_mapping;
#endif

static vec3 ssao_sample_kernel[8 * 8];
static vec3 ssao_noise[4 * 4];

#define SHADOW_MAP_RESOLUTION 512

#define OBJECT_COUNT 9
#define LIGHT_COUNT 32

typedef struct Resources {
    uint gbuffer;
    uint gtex_position;
    uint gtex_normal;
    uint gtex_albedo_spec;
    uint grbo_depth;

    uint tex_noise;
    uint fbo_ssao;
    uint tex_ssao;
    uint fbo_ssao_blur;
    uint tex_ssao_blur;

    vec3 object_positions[OBJECT_COUNT];
    vec3 light_positions[LIGHT_COUNT];
    vec3 light_colors[LIGHT_COUNT];

#if 0
    uint vao_skybox;

    vec3 light_position;
    uint vao_plane;
    uint vao_cube;

    uint vao_debug_quad;
    uint tex_depth_map;
    uint fbo_depth_map;
#endif
} Resources;
