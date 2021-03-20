#pragma once

#include "prelude.h"

#include "maths.h"

typedef struct Shader {
    uint program_id;
    uint vertex_id;
    /* uint geometry_id; */
    uint fragment_id;
} Shader;

Shader new_shader_from_source(char const *vertex_source, char const *fragment_source, Err *err);
Shader new_shader_from_filepath(char const *vertex_path, char const *fragment_path, Err *err);

bool reload_shader_from_source(
    Shader *shader, char const *vertex_source, char const *fragment_source);
bool reload_shader_from_filepath(
    Shader *shader, char const *vertex_path, char const *fragment_path);

void use_shader(Shader const shader);

void set_shader_int(Shader const shader, char const *name, int value);
void set_shader_bool(Shader const shader, char const *name, bool value);
void set_shader_float(Shader const shader, char const *name, f32 value);
void set_shader_vec2(Shader const shader, char const *name, vec2 const vec);
void set_shader_vec3(Shader const shader, char const *name, vec3 const vec);
void set_shader_vec4(Shader const shader, char const *name, vec4 const vec);
void set_shader_mat3(Shader const shader, char const *name, mat3 const mat);
void set_shader_mat4(Shader const shader, char const *name, mat4 const mat);
