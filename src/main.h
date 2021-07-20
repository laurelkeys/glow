#include "prelude.h"

//
// Header includes.
//

#include "camera.h"
#include "console.h"
#include "file.h"
#include "maths.h"
#include "model.h"
#include "opengl.h"
#include "shader.h"
#include "texture.h"
#include "vertices.h"
#include "window.h"

// Standard headers.
#include <stdio.h>

// External headers.
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <stb_image.h>

//
// Forward declarations.
//

void setup_shaders(void);
void process_input(GLFWwindow *window, f32 delta_time);
void set_window_callbacks(GLFWwindow *window);

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
    ShaderStrings paths;
    Shader shader;
} PathsToShader;

typedef struct PathToModel {
    char const *path;
    bool flip_on_load;
} PathToModel;

enum { BACKPACK = 0, NANOSUIT, CYBORG, PLANET, ROCK };

static PathToModel const choose_model[] = {
    [BACKPACK] = { GLOW_MODELS_ "backpack/backpack.obj", true },
    [NANOSUIT] = { GLOW_MODELS_ "nanosuit/nanosuit.obj", false },
    [CYBORG] = { GLOW_MODELS_ "cyborg/cyborg.obj", false },
    [PLANET] = { GLOW_MODELS_ "planet/planet.obj", false },
    [ROCK] = { GLOW_MODELS_ "rock/rock.obj", true },
};
