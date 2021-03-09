#pragma once

#include "prelude.h"

typedef struct Shader {
    uint program_id;
} Shader;

Shader
new_shader_from_source(char const *vertex_source, char const *fragment_source, Err *err);
Shader
new_shader_from_filepath(char const *vertex_path, char const *fragment_path, Err *err);

void use_shader(Shader const shader);

void set_shader_int(Shader const shader, char const *name, int value);
void set_shader_bool(Shader const shader, char const *name, bool value);
void set_shader_float(Shader const shader, char const *name, f32 value);
