#pragma once

#include "prelude.h"

#include "mesh.h"
#include "shader.h"

typedef struct Model {
    char const *path;
    Mesh *meshes; // @Ownership
    usize meshes_len;
    usize meshes_capacity;
} Model;

Model alloc_new_model_from_filepath(char const *model_path, Err *err);
void dealloc_model(Model *model);

void draw_model_with_shader(Model const *model, Shader const shader);
void draw_textureless_model_with_shader(Model const *model, Shader const shader);
