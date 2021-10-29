#include "file.h"
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

static void dealloc_texture_store(TextureStore *texture_store) {
    if (texture_store->paths) {
        for (usize i = 0; i < texture_store->len; ++i) { free(texture_store->paths[i]); }
        free(texture_store->paths);
        texture_store->paths = NULL;
    }

    free(texture_store->textures);
    texture_store->textures = NULL;
}

static usize count_assimp_material_textures_with_assimp_texture_types(
    struct aiMaterial const *ai_material,
    enum aiTextureType const ai_texture_types[],
    usize ai_texture_types_len) {
    usize count = 0;
    for (usize j = 0; j < ai_texture_types_len; ++j) {
        // @Note: a aiMaterial can have multiple textures.
        count += aiGetMaterialTextureCount(ai_material, ai_texture_types[j]);
    }
    return count;
}

static TextureStore alloc_texture_store_for_assimp_texture_types(
    struct aiScene const *ai_scene,
    enum aiTextureType const ai_texture_types[],
    usize ai_texture_types_len,
    Err *err) {
    if (*err) { return (TextureStore) { 0 }; }

    usize texture_store_capacity = 0;
    for (uint i = 0; i < ai_scene->mNumMaterials; ++i) {
        struct aiMaterial const *ai_material = ai_scene->mMaterials[i];
        texture_store_capacity += count_assimp_material_textures_with_assimp_texture_types(
            ai_material, ai_texture_types, ai_texture_types_len);
    }

    TextureStore texture_store = {
        .paths = calloc(texture_store_capacity, sizeof(char *)),
        .textures = calloc(texture_store_capacity, sizeof(Texture)),
        .len = 0,
        .capacity = texture_store_capacity,
    };

    if (!texture_store.paths || !texture_store.textures) {
        dealloc_texture_store(&texture_store);
        *err = Err_Calloc;
    }

    return texture_store;
}

// @Cleanup: this isn't great... maybe it could be specified as an arg when creating the model?
static enum aiTextureType const STORED_ASSIMP_TEXTURE_TYPES[] = {
    aiTextureType_DIFFUSE, aiTextureType_SPECULAR, aiTextureType_AMBIENT,
    aiTextureType_NORMALS, aiTextureType_HEIGHT,
};

static TextureMaterialType
texture_material_type_from_assimp_texture_type(enum aiTextureType ai_texture_type) {
    switch (ai_texture_type) {
        // @Volatile: keep in sync with TextureMaterialType.
        case aiTextureType_DIFFUSE: return TextureMaterialType_Diffuse;
        case aiTextureType_SPECULAR: return TextureMaterialType_Specular;
        case aiTextureType_AMBIENT: return TextureMaterialType_Ambient;
        case aiTextureType_NORMALS: return TextureMaterialType_Normal;
        case aiTextureType_HEIGHT: return TextureMaterialType_Height;
        default:
            GLOW_WARNING("unhandled assimp aiTextureType: `%d`", ai_texture_type);
            assert(false);
            return TextureMaterialType_None;
    }
}

static void store_textures_with_assimp_texture_type(
    TextureStore *texture_store,
    Str dir_path,
    struct aiMaterial const *ai_material,
    enum aiTextureType ai_texture_type,
    Err *err) {
    if (*err) { return; }

    uint const count = aiGetMaterialTextureCount(ai_material, ai_texture_type);
    for (uint i = 0; i < count; ++i) {
        struct aiString path = { 0 };
        if (aiGetMaterialTexture(
                ai_material, ai_texture_type, i, &path, NULL, NULL, NULL, NULL, NULL, NULL)
            != aiReturn_SUCCESS) {
            *err = Err_Assimp_Get_Texture;
            return;
        }

        // @Note: we assume all texture paths are relative to dir_path.
        usize const full_path_len = dir_path.len + 1 + path.length; // + 1 for the slash
        char *full_path = calloc(full_path_len + 1, sizeof(char));
        snprintf(full_path, full_path_len + 1, "%s" SLASH "%s", dir_path.data, &path.data[0]);

        DEFER(free(full_path)) {
            // Load and store the mesh texture together with its path so that
            // we can find it later on (when buillding the meshes themselves).
            assert(texture_store->len < texture_store->capacity);
            usize const len = texture_store->len;
            texture_store->len += 1;
            texture_store->paths[len] = alloc_str_copy(&path.data[0], err);
            texture_store->textures[len] = new_texture_from_filepath(
                full_path, (TextureSettings) { .generate_mipmap = true }, err);
            texture_store->textures[len].material_type =
                texture_material_type_from_assimp_texture_type(ai_texture_type);

            if (*err) {
                GLOW_WARNING("failed to load texture from path: `%s`", full_path);
            } else {
                GLOW_LOG("Loaded texture: `%s`", full_path);
            }
        }
    }
}

static bool load_stored_textures_with_assimp_texture_type_into_mesh_textures(
    Mesh *mesh,
    TextureStore const *texture_store,
    struct aiMaterial const *ai_material,
    enum aiTextureType ai_texture_type,
    Err *err) {
    if (*err) { return false; }

    bool found_all_textures = true;

    uint const count = aiGetMaterialTextureCount(ai_material, ai_texture_type);
    for (uint i = 0; i < count; ++i) {
        struct aiString path = { 0 };
        if (aiGetMaterialTexture(
                ai_material, ai_texture_type, i, &path, NULL, NULL, NULL, NULL, NULL, NULL)
            != aiReturn_SUCCESS) {
            *err = Err_Assimp_Get_Texture;
            return false;
        }

        // @Speed: find (and copy) the pre-loaded texture by comparing its path.
        bool found_it = false;
        for (usize j = 0; j < texture_store->len; ++j) {
            if (!strncmp(&path.data[0], texture_store->paths[j], path.length)) {
                mesh->textures[mesh->textures_len++] = texture_store->textures[j];
                found_it = true;
                break;
            }
        }
        if (!found_it) { GLOW_WARNING("could not find texture: `%s`", &path.data[0]); }
        found_all_textures = found_all_textures && found_it;
    }

    // @Note: this is more of a sanity check... but it might be good to set err in case it fails.
    assert(found_all_textures);
    return found_all_textures;
}

static Mesh alloc_mesh_from_assimp_mesh(
    TextureStore const *texture_store,
    struct aiMesh const *ai_mesh,
    struct aiScene const *ai_scene,
    Err *err) {
    if (*err) { return (Mesh) { 0 }; }

    struct aiMaterial *ai_material = ai_scene->mMaterials[ai_mesh->mMaterialIndex];
    assert(ai_material != NULL); // @Todo: can this be null? leaving a sanity check...

    usize const textures_capacity = count_assimp_material_textures_with_assimp_texture_types(
        ai_material, STORED_ASSIMP_TEXTURE_TYPES, ARRAY_LEN(STORED_ASSIMP_TEXTURE_TYPES));

    assert(textures_capacity <= texture_store->capacity);

    Mesh mesh = {
        .vertices = calloc(ai_mesh->mNumVertices, sizeof(Vertex)),
        .indices = calloc(3 * ai_mesh->mNumFaces, sizeof(uint)),
        .textures = calloc(textures_capacity, sizeof(Texture)),
    };

    if (!mesh.vertices || !mesh.indices || !mesh.textures) {
        dealloc_mesh(&mesh);
        *err = Err_Calloc;
        return (Mesh) { 0 };
    }

    //
    // Mesh vertices.
    //

    bool const has_texcoord = ai_mesh->mTextureCoords[0] != NULL;
    for (uint i = 0; i < ai_mesh->mNumVertices; ++i) {
        struct aiVector3D position = ai_mesh->mVertices[i];
        struct aiVector3D normal = ai_mesh->mNormals[i];
        struct aiVector3D texcoord =
            has_texcoord ? ai_mesh->mTextureCoords[0][i] : (struct aiVector3D) { 0 };
        /* struct aiVector3D tangent = ai_mesh->mTangents[i]; */
        /* struct aiVector3D bitangent = ai_mesh->mBitangents[i]; */
        mesh.vertices[mesh.vertices_len++] = (Vertex) {
            { position.x, position.y, position.z },
            { normal.x, normal.y, normal.z },
            { texcoord.x, texcoord.y },
        };
    }
    assert(mesh.vertices_len == ai_mesh->mNumVertices);

    //
    // Mesh indices.
    //

    // @Volatile: all faces are assumed to be triangular due to the aiProcess_Triangulate flag.
    for (uint i = 0; i < ai_mesh->mNumFaces; ++i) {
        assert(ai_mesh->mFaces[i].mNumIndices == 3);
        mesh.indices[mesh.indices_len++] = ai_mesh->mFaces[i].mIndices[0];
        mesh.indices[mesh.indices_len++] = ai_mesh->mFaces[i].mIndices[1];
        mesh.indices[mesh.indices_len++] = ai_mesh->mFaces[i].mIndices[2];
    }
    assert(mesh.indices_len == 3 * ai_mesh->mNumFaces);

    //
    // Mesh textures.
    //

    for (usize i = 0; i < ARRAY_LEN(STORED_ASSIMP_TEXTURE_TYPES); ++i) {
        load_stored_textures_with_assimp_texture_type_into_mesh_textures(
            &mesh,
            texture_store,
            ai_material,
            STORED_ASSIMP_TEXTURE_TYPES[i],
            err); // assert(!*err);
    }
    assert(mesh.textures_len == textures_capacity);

    //
    // Mesh VAO.
    //

    mesh.vao = create_mesh_vao(mesh.vertices, mesh.vertices_len, mesh.indices, mesh.indices_len);

    return mesh;
}

static void alloc_assimp_node_into_model_meshes(
    Model *model,
    TextureStore const *texture_store,
    struct aiNode const *ai_node,
    struct aiScene const *ai_scene,
    Err *err) {
    if (*err) { return; }

    // Process node's meshes.
    for (uint i = 0; i < ai_node->mNumMeshes; ++i) {
        struct aiMesh *ai_mesh = ai_scene->mMeshes[ai_node->mMeshes[i]];
        assert(model->meshes_len < model->meshes_capacity);
        model->meshes[model->meshes_len++] =
            alloc_mesh_from_assimp_mesh(texture_store, ai_mesh, ai_scene, err); // assert(!*err);
    }

    // Recursively process node's children.
    for (uint i = 0; i < ai_node->mNumChildren; ++i) {
        alloc_assimp_node_into_model_meshes(
            model, texture_store, ai_node->mChildren[i], ai_scene, err);
    }
}

static void
alloc_assimp_scene_into_model_meshes(Model *model, struct aiScene const *ai_scene, Err *err) {
    if (*err) { return; }

    TextureStore texture_store = alloc_texture_store_for_assimp_texture_types(
        ai_scene, STORED_ASSIMP_TEXTURE_TYPES, ARRAY_LEN(STORED_ASSIMP_TEXTURE_TYPES), err);

    char *dir_path = alloc_str_copy(model->path, err);

    if (*err == Err_None) {
        terminate_at_last_path_component_inplace(dir_path);
        Str const dir_path_str = { .data = dir_path, .len = strlen(dir_path) };

        // Pre-load aiMaterial textures.
        for (uint i = 0; i < ai_scene->mNumMaterials; ++i) {
            struct aiMaterial const *ai_material = ai_scene->mMaterials[i];
            for (usize j = 0; j < ARRAY_LEN(STORED_ASSIMP_TEXTURE_TYPES); ++j) {
                store_textures_with_assimp_texture_type(
                    &texture_store,
                    dir_path_str,
                    ai_material,
                    STORED_ASSIMP_TEXTURE_TYPES[j],
                    err); // assert(!*err);
                assert(texture_store.len <= texture_store.capacity);
            }
        }
        assert(texture_store.len == texture_store.capacity);

        // Convert assimp meshes.
        alloc_assimp_node_into_model_meshes(
            model, &texture_store, ai_scene->mRootNode, ai_scene, err);
        assert(model->meshes_len == model->meshes_capacity);
    }

    free(dir_path);

    // Clean up the temporary mesh texture data allocated to create model->meshes.
    dealloc_texture_store(&texture_store);
}

/* clang-format off */
static uint const POST_PROCESS_FLAGS = 0
    | aiProcess_Triangulate // @Volatile: `alloc_mesh_from_assimp_mesh` relies on this
    | aiProcess_SortByPType
    | aiProcess_GenUVCoords
    | aiProcess_FindInstances
    | aiProcess_OptimizeMeshes
    | aiProcess_GenSmoothNormals
    | aiProcess_CalcTangentSpace
    | aiProcess_PreTransformVertices
    | aiProcess_JoinIdenticalVertices
    | aiProcess_ValidateDataStructure;
/* clang-format on */

// @Temporary: used for logging.
typedef struct CountOfAssimpScene {
    uint nodes;
    uint indices;
    uint vertices;
} CountOfAssimpScene;

static CountOfAssimpScene
count_assimp_scene(struct aiScene const *ai_scene, struct aiNode const *ai_node) {
    CountOfAssimpScene count = { .nodes = 1 };

    for (uint i = 0; i < ai_node->mNumMeshes; ++i) {
        struct aiMesh *ai_mesh = ai_scene->mMeshes[ai_node->mMeshes[i]];
        count.indices += ai_mesh->mNumFaces * ai_mesh->mFaces[0].mNumIndices;
        count.vertices += ai_mesh->mNumVertices;
    }

    for (uint i = 0; i < ai_node->mNumChildren; ++i) {
        CountOfAssimpScene const child_count =
            count_assimp_scene(ai_scene, ai_node->mChildren[i]);
        count.nodes += child_count.nodes;
        count.indices += child_count.indices;
        count.vertices += child_count.vertices;
    }

    return count;
}

// @Temporary: used for logging.
typedef struct CountOfModel {
    usize meshes;
    usize indices;
    usize vertices;
    usize textures;
    usize materials; // @Note: filters out TextureMaterialType_None
} CountOfModel;

static CountOfModel count_model(Model const *model) {
    CountOfModel count = { .meshes = model->meshes_len };

    Mesh const *meshes = model->meshes;
    for (Mesh const *mesh = &meshes[0]; mesh != &meshes[model->meshes_len]; ++mesh) {
        count.indices += mesh->indices_len;
        count.vertices += mesh->vertices_len;
        count.textures += mesh->textures_len;
        for (usize i = 0; i < mesh->textures_len; ++i) {
            count.materials +=
                (mesh->textures[i].material_type != TextureMaterialType_None) ? 1 : 0;
        }
    }

    return count;
}

Model alloc_new_model_from_filepath_using_assimp(char const *path, Err *err) {
    struct aiScene const *ai_scene = aiImportFile(path, POST_PROCESS_FLAGS);

    if (!ai_scene || !ai_scene->mRootNode || (ai_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)) {
        GLOW_WARNING("assimp import failed with: `%s`", aiGetErrorString());
        *err = Err_Assimp_Import;
        return (Model) { 0 };
    }

#ifndef NDEBUG
    for (usize i = 0; i < ai_scene->mNumMaterials; ++i) {
        struct aiString name = { 0 };
        if (aiGetMaterialString(ai_scene->mMaterials[i], AI_MATKEY_NAME, &name)
            != aiReturn_SUCCESS) {
            assert(false);
        }
        GLOW_LOG("Material name: `%s`", name.data);
    }
#endif

    Model model = {
        .path = path,
        .meshes = calloc(ai_scene->mNumMeshes, sizeof(Mesh)),
        .meshes_len = 0,
        .meshes_capacity = ai_scene->mNumMeshes,
    };

    if (!model.meshes) { *err = Err_Calloc; }
    alloc_assimp_scene_into_model_meshes(&model, ai_scene, err);

#ifndef NDEBUG
    {
        CountOfAssimpScene const count_of = count_assimp_scene(ai_scene, ai_scene->mRootNode);
        GLOW_LOG(
            "(aiScene) meshes, indices, vertices, textures, materials, nodes = %d",
            ai_scene->mNumMeshes,
            count_of.indices,
            count_of.vertices,
            ai_scene->mNumTextures,
            ai_scene->mNumMaterials,
            count_of.nodes);
        assert(ai_scene->mNumMeshes == count_of.nodes - 1);
    }
    {
        CountOfModel const count_of = count_model(&model);
        GLOW_LOG(
            "(Model) meshes, indices, vertices, textures, materials = %d",
            count_of.meshes,
            count_of.indices,
            count_of.vertices,
            count_of.textures,
            count_of.materials);
    }
#endif

    // Clean up assimp scene data.
    aiReleaseImport(ai_scene);

    return model;
}
