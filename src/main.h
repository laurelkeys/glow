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
// Resource loading macros.
//

#define TRY_NEW_SHADER(shader_strings_path, err_ptr)        \
    new_shader_from_filepath(shader_strings_path, err_ptr); \
    if (*(err_ptr)) { goto main_err; }

#define TRY_NEW_TEXTURE(filename, err_ptr)                       \
    new_texture_from_filepath(GLOW_TEXTURES_ filename, err_ptr); \
    if (*(err_ptr)) { goto main_err; }

#define TRY_ALLOC_NEW_MODEL(filename, err_ptr)                     \
    alloc_new_model_from_filepath(GLOW_MODELS_ filename, err_ptr); \
    if (*(err_ptr)) { goto main_err; }

//
// Input processing macros.
//

#define IS_PRESSED(key) (glfwGetKey(window, GLFW_KEY_##key) == GLFW_PRESS)

#define ON_PRESS(key, is_key_pressed)                                              \
    (glfwGetKey(window, GLFW_KEY_##key) != GLFW_PRESS) { is_key_pressed = false; } \
    else if (!is_key_pressed && (is_key_pressed = true))
