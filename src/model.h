#pragma once

#include "prelude.h"

// Forward declarations.
typedef struct Mesh Mesh;
typedef struct Shader Shader;

typedef struct Model {
    char const *path;
    Mesh *meshes; // @Ownership
    usize meshes_len;
    usize meshes_capacity;
} Model;

Model alloc_new_model_from_filepath(char const *path, Err *err);
void dealloc_model(Model *model);

void draw_model_direct(Model const *model);
void draw_model_with_shader(Model const *model, Shader const *shader);
void draw_model_textureless_with_shader(Model const *model, Shader const *shader);
