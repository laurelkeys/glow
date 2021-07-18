#include "model.h"

#include "console.h"
#include "file.h"
#include "mesh.h"
#include "shader.h"
#include "texture.h"

#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

// @Note: this is a helper structure used to hold together texture
// data that we only need temporarily (while building a new Model).
typedef struct TextureStore {
    char **paths; // @Ownership
    Texture *textures; // @Ownership
    usize len;
    usize capacity;
} TextureStore;

static int const STORED_ASSIMP_TYPES[] = {
    aiTextureType_DIFFUSE, // @Temporary: the simple_texture shader only has a diffuse sampler2D!
    // aiTextureType_DIFFUSE, aiTextureType_SPECULAR, aiTextureType_HEIGHT, aiTextureType_AMBIENT
};

static TextureMaterialType material_type_from_assimp(enum aiTextureType type) {
    switch (type) {
        // @Volatile: :SyncWithTextureMaterialType:
        case aiTextureType_DIFFUSE: return TextureMaterialType_Diffuse;
        case aiTextureType_SPECULAR: return TextureMaterialType_Specular;
        case aiTextureType_HEIGHT: return TextureMaterialType_Normal;
        case aiTextureType_AMBIENT: return TextureMaterialType_Height;
        default:
            GLOW_WARNING("unhandled assimp aiTextureType: `%d`", type);
            assert(false);
            return TextureMaterialType_None;
    }
}

static void store_textures_with_assimp_type(
    TextureStore *texture_store,
    str dir_path,
    struct aiMaterial const *material,
    enum aiTextureType type,
    Err *err) {
    uint const count = aiGetMaterialTextureCount(material, type);
    for (uint i = 0; i < count; ++i) {
        struct aiString path = { 0 };
        if (aiGetMaterialTexture(material, type, i, &path, NULL, NULL, NULL, NULL, NULL, NULL)
            != aiReturn_SUCCESS) {
            *err = Err_Assimp_Get_Texture;
            return;
        }

        // @Note: we assume all texture paths are relative to dir_path.
        usize const full_path_len = dir_path.len + 1 + path.length; // + 1 for the slash
        char *full_path = calloc(full_path_len + 1, sizeof(char));
        DEFER(free(full_path)) {
            snprintf(full_path, full_path_len + 1, "%s" SLASH "%s", dir_path.data, &path.data[0]);

            // Load and store the mesh texture together with its path so that
            // we can find it later on (when buillding the meshes themselves).
            assert(texture_store->len < texture_store->capacity);
            usize const len = texture_store->len;
            texture_store->len += 1;
            texture_store->paths[len] = alloc_str_copy(&path.data[0]);
            texture_store->textures[len] = new_texture_from_filepath(full_path, err);
            texture_store->textures[len].material = material_type_from_assimp(type);

            if (*err) {
                GLOW_WARNING("failed to load texture from path: `%s`", full_path);
            } else {
                GLOW_LOG("Loaded texture: `%s`", full_path);
            }
        }
    }
}

static bool fetch_stored_textures_with_assimp_type(
    Texture *textures,
    usize *textures_len,
    TextureStore const *texture_store,
    struct aiMaterial const *material,
    enum aiTextureType type,
    Err *err) {
    bool found_all_textures = true;

    uint const count = aiGetMaterialTextureCount(material, type);
    for (uint i = 0; i < count; ++i) {
        struct aiString path = { 0 };
        if (aiGetMaterialTexture(material, type, i, &path, NULL, NULL, NULL, NULL, NULL, NULL)
            != aiReturn_SUCCESS) {
            return (*err = Err_Assimp_Get_Texture, false);
        }

        // Find (and copy) the pre-loaded texture by comparing its path.
        bool found_it = false;
        for (usize j = 0; j < texture_store->len; ++j) {
            if (!strncmp(&path.data[0], texture_store->paths[j], path.length)) {
                found_it = true;
                textures[(*textures_len)++] = texture_store->textures[j];
            }
        }
        if (!found_it) { GLOW_WARNING("could not find texture: `%s`", &path.data[0]); }
        found_all_textures = found_all_textures && found_it;
    }

    // @Note: this is more of a sanity check... but it might be good to set err in case it fails.
    assert(found_all_textures);
    return found_all_textures;
}

static Mesh process_assimp_mesh_into_alloc_mesh(
    TextureStore const *texture_store,
    struct aiMesh const *mesh,
    struct aiScene const *scene,
    Err *err) {
    Vertex *vertices = calloc(mesh->mNumVertices, sizeof(Vertex));

    // @Volatile: all faces are assumed to be triangular due to the aiProcess_Triangulate flag.
    uint *indices = calloc(3 * mesh->mNumFaces, sizeof(uint));

    struct aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
    usize textures_capacity = 0;
    for (usize i = 0; i < ARRAY_LEN(STORED_ASSIMP_TYPES); ++i) {
        textures_capacity += aiGetMaterialTextureCount(material, STORED_ASSIMP_TYPES[i]);
    }
    assert(textures_capacity <= texture_store->capacity);
    Texture *textures = calloc(textures_capacity, sizeof(Texture));

    if (!vertices || !indices || !textures) {
        free(vertices);
        free(indices);
        free(textures);
        return (*err = Err_Calloc, (Mesh) { 0 });
    }

    //
    // Mesh vertices.
    //

    bool const has_texcoord = (mesh->mTextureCoords[0] != NULL);

    if (has_texcoord) {
        for (uint i = 0; i < mesh->mNumVertices; ++i) {
            struct aiVector3D position = mesh->mVertices[i];
            struct aiVector3D normal = mesh->mNormals[i];
            struct aiVector3D texcoord = mesh->mTextureCoords[0][i];
            /* struct aiVector3D tangent = mesh->mTangents[i]; */
            /* struct aiVector3D bitangent = mesh->mBitangents[i]; */
            vertices[i] = (Vertex) {
                { position.x, position.y, position.z },
                { normal.x, normal.y, normal.z },
                { texcoord.x, texcoord.y },
            };
        }
    } else {
        for (uint i = 0; i < mesh->mNumVertices; ++i) {
            struct aiVector3D position = mesh->mVertices[i];
            struct aiVector3D normal = mesh->mNormals[i];
            vertices[i] = (Vertex) {
                { position.x, position.y, position.z },
                { normal.x, normal.y, normal.z },
            };
        }
    }

    //
    // Mesh indices.
    //

    for (uint i = 0; i < mesh->mNumFaces; ++i) {
        assert(mesh->mFaces[i].mNumIndices == 3);
        indices[3 * i + 0] = mesh->mFaces[i].mIndices[0];
        indices[3 * i + 1] = mesh->mFaces[i].mIndices[1];
        indices[3 * i + 2] = mesh->mFaces[i].mIndices[2];
    }

    //
    // Mesh textures.
    //

    usize textures_len = 0;
    for (usize i = 0; i < ARRAY_LEN(STORED_ASSIMP_TYPES); ++i) {
        fetch_stored_textures_with_assimp_type(
            textures, &textures_len, texture_store, material, STORED_ASSIMP_TYPES[i], err);
        assert(!*err);
    }
    assert(textures_len == textures_capacity);

    return new_mesh(
        vertices,
        /*vertices_len*/ mesh->mNumVertices,
        indices,
        /*indices_len*/ 3 * mesh->mNumFaces,
        textures,
        textures_len);
}

static void process_assimp_node_to_alloc_model_meshes(
    Model *model,
    TextureStore const *texture_store,
    struct aiNode const *node,
    struct aiScene const *scene,
    Err *err) {
    // Process node's meshes.
    for (uint i = 0; i < node->mNumMeshes; ++i) {
        struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        assert(model->meshes_len < model->meshes_capacity);
        model->meshes[(model->meshes_len)++] =
            process_assimp_mesh_into_alloc_mesh(texture_store, mesh, scene, err);
    }

    // Recursively process node's children.
    for (uint i = 0; i < node->mNumChildren; ++i) {
        process_assimp_node_to_alloc_model_meshes(
            model, texture_store, node->mChildren[i], scene, err);
    }
}

// clang-format off
static uint const POST_PROCESS_FLAGS = 0
    | aiProcess_ValidateDataStructure
    | aiProcess_Triangulate // @Volatile: `process_assimp_mesh_to_alloc_mesh` relies on this
    | aiProcess_FlipUVs
    | aiProcess_JoinIdenticalVertices
    | aiProcess_CalcTangentSpace
    | aiProcess_GenSmoothNormals
    | aiProcess_GenUVCoords
    | aiProcess_OptimizeMeshes
    | aiProcess_SortByPType;
// clang-format on

static usize count_assimp_nodes(struct aiNode const *node) {
    usize count = 1;
    for (uint i = 0; i < node->mNumChildren; ++i) {
        count += count_assimp_nodes(node->mChildren[i]);
    }
    return count;
}

Model alloc_new_model_from_filepath(char const *model_path, Err *err) {
    struct aiScene const *scene = aiImportFile(model_path, POST_PROCESS_FLAGS);

    if (!scene || !scene->mRootNode || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)) {
        GLOW_WARNING("assimp import failed with: `%s`", aiGetErrorString());
        return (*err = Err_Assimp_Import, (Model) { 0 });
    }

    GLOW_LOG("Loading model: `%s`", model_path);
    assert(count_assimp_nodes(scene->mRootNode) - 1 == scene->mNumMeshes);

    assert(scene->mNumMeshes > 0);
    Model model = {
        .path = model_path,
        .meshes = calloc(scene->mNumMeshes, sizeof(Mesh)),
        .meshes_len = 0,
        .meshes_capacity = scene->mNumMeshes,
    };

    assert(scene->mNumMaterials > 0);
    usize texture_store_capacity = 0;
    for (uint i = 0; i < scene->mNumMaterials; ++i) {
        // @Note: a material can have multiple textures.
        struct aiMaterial const *material = scene->mMaterials[i];
        for (usize j = 0; j < ARRAY_LEN(STORED_ASSIMP_TYPES); ++j) {
            texture_store_capacity += aiGetMaterialTextureCount(material, STORED_ASSIMP_TYPES[j]);
        }
    }
    TextureStore texture_store = {
        .paths = calloc(texture_store_capacity, sizeof(char *)),
        .textures = calloc(texture_store_capacity, sizeof(Texture)),
        .len = 0,
        .capacity = texture_store_capacity,
    };

    if (!model.meshes || !texture_store.paths || !texture_store.textures) {
        *err = Err_Calloc;
    } else {
        char *dir_path = alloc_str_copy(model_path);
        DEFER(free(dir_path)) {
            terminate_at_last_path_component_inplace(dir_path);
            str const dir_path_str = { .data = dir_path, .len = strlen(dir_path) };

            // Pre-load material textures.
            for (uint i = 0; i < scene->mNumMaterials; ++i) {
                struct aiMaterial const *material = scene->mMaterials[i];
                for (usize j = 0; j < ARRAY_LEN(STORED_ASSIMP_TYPES); ++j) {
                    store_textures_with_assimp_type(
                        &texture_store, dir_path_str, material, STORED_ASSIMP_TYPES[j], err);
                    assert(texture_store.len <= texture_store.capacity);
                    assert(!*err);
                }
            }
            assert(texture_store.len == texture_store.capacity);
        }

        // Convert assimp meshes.
        if (!*err) {
            process_assimp_node_to_alloc_model_meshes(
                &model, &texture_store, scene->mRootNode, scene, err);
            assert(model.meshes_len == model.meshes_capacity);
        }

        // Clean up the temporary mesh texture data used to create model.meshes.
        for (usize i = 0; i < texture_store.len; ++i) { free(texture_store.paths[i]); }
        free(texture_store.paths);
        free(texture_store.textures);
    }

    // Clean up assimp scene data.
    aiReleaseImport(scene);

    if (*err) {
        GLOW_WARNING("failed to load `%s` model", point_at_last_path_component(model_path));
    } else {
        GLOW_LOG("Finished loading `%s` model", point_at_last_path_component(model_path));
    }

    return model;
}

void dealloc_model(Model *model) {
    for (usize i = 0; i < model->meshes_len; ++i) { dealloc_mesh(&model->meshes[i]); }
    free(model->meshes);
}

void draw_model_with_shader(Model const *model, Shader const *shader) {
    for (usize i = 0; i < model->meshes_len; ++i) {
        draw_mesh_with_shader(&model->meshes[i], shader);
    }
}

void draw_textureless_model_with_shader(Model const *model, Shader const *shader) {
    for (usize i = 0; i < model->meshes_len; ++i) {
        // @Hack: ignore textures while drawing the mesh.
        usize const textures_len = model->meshes[i].textures_len;
        DEFER(model->meshes[i].textures_len = textures_len) {
            model->meshes[i].textures_len = 0;
            draw_mesh_with_shader(&model->meshes[i], shader);
        }
    }
}
