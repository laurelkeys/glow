#include "model.h"

#include "console.h"
#include "file.h"

#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

static bool fetch_from_preloaded_textures_with_type(
    Texture *textures,
    usize *textures_len,
    usize textures_capacity,
    Model const *model,
    struct aiMaterial const *material,
    enum aiTextureType type,
    Err *err) {
    bool found_all_textures = true;

    uint const count = aiGetMaterialTextureCount(material, type);
    for (usize i = 0; i < count; ++i) {
        struct aiString path = { 0 };
        if (aiGetMaterialTexture(material, type, i, &path, NULL, NULL, NULL, NULL, NULL, NULL)
            != aiReturn_SUCCESS) {
            return (*err = Err_Assimp_Get_Texture, false);
        }

        // Find (and copy) the pre-loaded texture by comparing its path.
        bool found_it = false;
        for (usize j = 0; j < model->mesh_textures_len; ++j) {
            if (!strncmp(&path.data[0], model->mesh_textures_paths[j], path.length)) {
                found_it = true;
                assert(*textures_len < textures_capacity);
                textures[(*textures_len)++] = model->mesh_textures[j];
            }
        }
        found_all_textures = found_all_textures && found_it;
    }

    // @Note: this is more of a sanity check... but it might be good to set err in case it fails.
    assert(found_all_textures);
    return found_all_textures;
}

static Mesh process_mesh(
    Model const *model, struct aiMesh const *mesh, struct aiScene const *scene, Err *err) {
    //
    // Mesh vertices.
    //

    Vertex *vertices = calloc(mesh->mNumVertices, sizeof(Vertex));
    if (!vertices) { return (*err = Err_Calloc, (Mesh) { 0 }); }

    for (uint i = 0; i < mesh->mNumVertices; ++i) {
        struct aiVector3D position = mesh->mVertices[i];
        struct aiVector3D normal = mesh->mNormals[i];
        struct aiVector3D texcoords = (mesh->mTextureCoords[0] != NULL)
                                          ? mesh->mTextureCoords[0][i]
                                          : (struct aiVector3D) { 0 };
        vertices[i] = (Vertex) {
            { position.x, position.y, position.z },
            { normal.x, normal.y, normal.z },
            { texcoords.x, texcoords.y },
        };
    }

    //
    // Mesh indices.
    //

    // @Volatile: all faces are assumed to be triangular due to the aiProcess_Triangulate flag.
    uint *indices = calloc(3 * mesh->mNumFaces, sizeof(uint));
    if (!indices) {
        free(vertices);
        return (*err = Err_Calloc, (Mesh) { 0 });
    }

    for (uint i = 0; i < mesh->mNumFaces; ++i) {
        assert(mesh->mFaces[i].mNumIndices == 3);
        indices[3 * i + 0] = mesh->mFaces[i].mIndices[0];
        indices[3 * i + 1] = mesh->mFaces[i].mIndices[1];
        indices[3 * i + 2] = mesh->mFaces[i].mIndices[2];
    }

    //
    // Mesh textures.
    //

    struct aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
    uint const diffuse_len = aiGetMaterialTextureCount(material, aiTextureType_DIFFUSE);
    uint const specular_len = aiGetMaterialTextureCount(material, aiTextureType_SPECULAR);

    // @Speed: currently a Texture is no larger than two ints (and uint plus an enum),
    // so it's cheap enough to copy. But if it ever gets larger, it'd be better to store
    // texture handles inside of Mesh instead (i.e. usize indices into the model's array).
    usize const textures_capacity = diffuse_len + specular_len;
    assert(textures_capacity <= model->mesh_textures_capacity);
    Texture *textures = calloc(textures_capacity, sizeof(Texture));
    if (!textures) {
        free(vertices);
        free(indices);
        return (*err = Err_Calloc, (Mesh) { 0 });
    }

#define FETCH_TEXTURES_WITH_TYPE(material, type, textures_len_ptr) \
    fetch_from_preloaded_textures_with_type(                       \
        textures, textures_len_ptr, textures_capacity, model, material, type, err)

    usize textures_len = 0;
    FETCH_TEXTURES_WITH_TYPE(material, aiTextureType_DIFFUSE, &textures_len);
    FETCH_TEXTURES_WITH_TYPE(material, aiTextureType_SPECULAR, &textures_len);
    assert(textures_len == textures_capacity);

#undef FETCH_TEXTURES_WITH_TYPE

    return new_mesh(
        vertices,
        /*vertices_len*/ mesh->mNumVertices,
        indices,
        /*indices_len*/ 3 * mesh->mNumFaces,
        textures,
        textures_len);
}

static bool
process_node(Model *model, struct aiNode const *node, struct aiScene const *scene, Err *err) {
    for (uint i = 0; !*err && i < node->mNumMeshes; ++i) {
        struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        assert(model->meshes_len < model->meshes_capacity);
        model->meshes[(model->meshes_len)++] = process_mesh(model, mesh, scene, err);
    }

    for (uint i = 0; !*err && i < node->mNumChildren; ++i) {
        process_node(model, node->mChildren[i], scene, err);
    }

    return *err == Err_None;
}

static TextureType mesh_texture_type_from_assimp(enum aiTextureType type) {
    switch (type) {
        case aiTextureType_DIFFUSE: return TextureType_Diffuse;
        case aiTextureType_SPECULAR: return TextureType_Specular;
        default:
            GLOW_WARNING("unhandled assimp aiTextureType: `%d`", type);
            assert(false);
            return TextureType_None;
    }
}

static bool preload_textures_with_type(
    Model *model,
    usize dir_path_len,
    struct aiMaterial const *material,
    enum aiTextureType type,
    Err *err) {
    uint const count = aiGetMaterialTextureCount(material, type);
    for (usize i = 0; i < count; ++i) {
        struct aiString path = { 0 };
        if (aiGetMaterialTexture(material, type, i, &path, NULL, NULL, NULL, NULL, NULL, NULL)
            != aiReturn_SUCCESS) {
            return (*err = Err_Assimp_Get_Texture, false);
        }

        // @Note: we assume all texture paths are relative to dir_path.
        usize const full_path_len = dir_path_len + 1 + path.length; // + 1 for the slash
        char *full_path = calloc(full_path_len + 1, sizeof(char));
#ifdef _WIN32
        snprintf(full_path, full_path_len + 1, "%s\\%s", model->dir_path, &path.data[0]);
#else
        snprintf(full_path, full_path_len + 1, "%s/%s", model->dir_path, &path.data[0]);
#endif

        // Load and store the mesh texture, together with its path so that
        // we can find it later on (when buillding the meshes themselves).
        usize const mesh_textures_len = model->mesh_textures_len;
        {
            assert(mesh_textures_len < model->mesh_textures_capacity);
            model->mesh_textures_paths[mesh_textures_len] = alloc_str_copy(&path.data[0]);
            model->mesh_textures[mesh_textures_len] = new_texture_from_filepath(full_path, err);
            model->mesh_textures[mesh_textures_len].type = mesh_texture_type_from_assimp(type);
        }
        model->mesh_textures_len += 1;

        if (*err) {
            GLOW_WARNING("failed to load texture: `%s`", full_path);
        } else {
            GLOW_LOG("Loaded texture: `%s`", full_path);
        }
    }

    return *err == Err_None;
}

Model alloc_new_model_from_filepath(char const *model_path, Err *err) {
    uint post_process_flags = 0;
    post_process_flags |= aiProcess_ValidateDataStructure;
    post_process_flags |= aiProcess_Triangulate; // @Volatile: `process_mesh` relies on this
    post_process_flags |= aiProcess_FlipUVs;
    post_process_flags |= aiProcess_JoinIdenticalVertices;
    // post_process_flags |= aiProcess_CalcTangentSpace;
    // post_process_flags |= aiProcess_GenSmoothNormals;

    struct aiScene const *scene = aiImportFile(model_path, post_process_flags);

    if (!scene || !scene->mRootNode || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)) {
        GLOW_WARNING("assimp import failed with: `%s`", aiGetErrorString());
        return (*err = Err_Assimp_Import, (Model) { 0 });
    }

    GLOW_LOG("Loading model: `%s`", model_path);
    char *dir_path = alloc_str_copy(model_path);
    terminate_at_last_path_component(dir_path); // modifies dir_path in-place

    Model model = {
        .dir_path = dir_path,

        .meshes = calloc(scene->mNumMeshes, sizeof(Mesh)),
        .meshes_len = 0,
        .meshes_capacity = scene->mNumMeshes,

        // @Note: we want to use mNumMaterials in here, not mNumTextures!
        .mesh_textures_paths = calloc(scene->mNumMaterials, sizeof(char *)),
        .mesh_textures = calloc(scene->mNumMaterials, sizeof(Texture)),
        .mesh_textures_len = 0,
        .mesh_textures_capacity = scene->mNumMaterials,
    };

    if (model.meshes_capacity > 0 && !model.meshes) { *err = Err_Calloc; }
    if (model.mesh_textures_capacity > 0
        && (!model.mesh_textures_paths || !model.mesh_textures)) {
        *err = Err_Calloc;
    }

    if (!*err) {
        // Pre-load textures.
        usize const dir_path_len = strlen(dir_path);

#define LOAD_TEXTURES_WITH_TYPE(material, type) \
    preload_textures_with_type(&model, dir_path_len, material, type, err)

        for (uint i = 0; i < scene->mNumMaterials; ++i) {
            struct aiMaterial const *material = scene->mMaterials[i];
            LOAD_TEXTURES_WITH_TYPE(material, aiTextureType_DIFFUSE);
            LOAD_TEXTURES_WITH_TYPE(material, aiTextureType_SPECULAR);
        }
        assert(model.mesh_textures_len == model.mesh_textures_capacity);

#undef LOAD_TEXTURES_WITH_TYPE

        // Convert assimp meshes.
        if (!*err) {
            process_node(&model, scene->mRootNode, scene, err);
            assert(model.meshes_len == model.meshes_capacity);
        }
    }

    aiReleaseImport(scene);

    if (*err) {
        GLOW_WARNING("failed to load model");
    } else {
        GLOW_LOG("Finished loading `%s` model", point_at_last_path_component(model_path));
    }

    return model; // @Speed: is it worth it using a Model pointer to avoid returning this copy?
}

void dealloc_model(Model *model) {
    free(model->dir_path);

    for (usize i = 0; i < model->meshes_len; ++i) { dealloc_mesh(&model->meshes[i]); }
    free(model->meshes);

    for (usize i = 0; i < model->mesh_textures_len; ++i) { free(model->mesh_textures_paths[i]); }
    free(model->mesh_textures_paths);
    free(model->mesh_textures);
}

void draw_model_with_shader(Model const *model, Shader const shader) {
    for (usize i = 0; i < model->meshes_len; ++i) {
        draw_mesh_with_shader(&model->meshes[i], shader);
    }
}
