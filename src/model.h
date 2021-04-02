#pragma once

#include "prelude.h"

#include "mesh.h"
#include "shader.h"

typedef struct Model {
    char const *dir_path;
    Mesh *meshes;
    usize meshes_len;
#if 0
    Texture *_textures_store;
    usize _textures_store_len;
#endif
} Model;

Model alloc_new_model_from_filepath(char const *model_path, Err *err);

void draw_model_with_shader(Model const *model, Shader const shader);
