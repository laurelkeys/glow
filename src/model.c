#include "model.h"

#include "console.h"
#include "mesh.h"

Model alloc_new_model_from_filepath_using_assimp(char const *path, Err *err);
/* Model alloc_new_model_from_filepath_using_cgltf(char const *path, Err *err);
Model alloc_new_model_from_filepath_using_fast_obj(char const *path, Err *err); */

#include "model_assimp.inl"
/* #include "model_cgltf.inl"
#include "model_fast_obj.inl" */

Model alloc_new_model_from_filepath(char const *path, Err *err) {
    if (*err) { return (Model) { 0 }; }

    GLOW_LOG("Loading model: `%s`", path);

    // @Todo: depending on the path extension, choose cgltf/fast_obj instead.
    Model const model = alloc_new_model_from_filepath_using_assimp(path, err);

    if (*err) {
        GLOW_WARNING("failed to load `%s` model", point_at_last_path_component(model.path));
    } else {
        GLOW_LOG("Finished loading `%s` model", point_at_last_path_component(model.path));
    }

    return model;
}

void dealloc_model(Model *model) {
    if (model->meshes) {
        for (usize i = 0; i < model->meshes_len; ++i) {
            destroy_mesh_vao(&model->meshes[i]);
            dealloc_mesh(&model->meshes[i]);
        }
        free(model->meshes);
        model->meshes = NULL;
    }
}

void draw_model_direct(Model const *model) {
    for (usize i = 0; i < model->meshes_len; ++i) { draw_mesh_direct(&model->meshes[i]); }
}

void draw_model_with_shader(Model const *model, Shader const *shader) {
    for (usize i = 0; i < model->meshes_len; ++i) {
        draw_mesh_with_shader(&model->meshes[i], shader);
    }
}

void draw_model_textureless_with_shader(Model const *model, Shader const *shader) {
    for (usize i = 0; i < model->meshes_len; ++i) {
        // @Hack: ignore textures while drawing the mesh.
        usize const textures_len = model->meshes[i].textures_len;

        model->meshes[i].textures_len = 0;
        draw_mesh_with_shader(&model->meshes[i], shader);
        model->meshes[i].textures_len = textures_len;
    }
}
