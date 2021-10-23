#pragma once

#include "prelude.h"

#include "maths_types.h"

typedef struct Shader {
    uint program_id;
} Shader;

// @Note: geometry may be null, but vertex and fragment are assumed not to be.
typedef struct ShaderStrings {
    char const *vertex;
    char const *fragment;
    char const *geometry;
} ShaderStrings;

Shader new_shader_from_source(ShaderStrings const source, Err *err);
Shader new_shader_from_filepath(ShaderStrings const path, Err *err);

bool try_reload_shader_from_source(Shader *shader, ShaderStrings const source);
bool try_reload_shader_from_filepath(Shader *shader, ShaderStrings const path);

void use_shader(Shader const shader);

void set_shader_int(Shader const shader, char const *name, int value);
void set_shader_bool(Shader const shader, char const *name, bool value);
void set_shader_float(Shader const shader, char const *name, f32 value);
void set_shader_sampler2D(Shader const shader, char const *name, uint texture_unit);
void set_shader_vec2(Shader const shader, char const *name, vec2 const vec);
void set_shader_vec3(Shader const shader, char const *name, vec3 const vec);
void set_shader_vec4(Shader const shader, char const *name, vec4 const vec);
void set_shader_mat3(Shader const shader, char const *name, mat3 const mat);
void set_shader_mat4(Shader const shader, char const *name, mat4 const mat);
