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

static uint count_assimp_material_textures_with_types(
    struct aiMaterial const *ai_material,
    enum aiTextureType const ai_texture_types[],
    usize ai_texture_types_len) {
    uint count = 0;
    for (usize i = 0; i < ai_texture_types_len; ++i) {
        count += aiGetMaterialTextureCount(ai_material, ai_texture_types[i]);
    }
    return count;
}

// @Refactor: instead of passing an array of assimp types that should be stored,
// we could pass a bitmask of the texture material types that we want to add to
// a given mesh (and move the mapping from between enum types to this function).
static TextureStore alloc_texture_store_for_assimp_texture_types(
    struct aiScene const *ai_scene,
    enum aiTextureType const ai_texture_types[],
    usize ai_texture_types_len,
    Err *err) {
    if (*err) { return (TextureStore) { 0 }; }

    usize texture_store_capacity = 0;
    for (uint i = 0; i < ai_scene->mNumMaterials; ++i) {
        texture_store_capacity += count_assimp_material_textures_with_types(
            ai_scene->mMaterials[i], ai_texture_types, ai_texture_types_len);
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

static TextureSettings const SETTINGS_FOR_TEXTURE_STORE = {
    .format = TextureFormat_Default,
    .apply_srgb_eotf = true, // @Fixme: this should only be set for color textures!
    .highp_bitdepth = false,
    .floating_point = false,
    .generate_mipmap = true,
};

static void store_texture_with_assimp_material_texture_type_index(
    TextureStore *texture_store,
    Str const dir_path_str,
    struct aiMaterial const *ai_material,
    enum aiTextureType const ai_texture_type,
    uint index,
    Err *err) {
    if (*err) { return; }

    struct aiString path = { 0 };
    if (aiGetMaterialTexture(
            ai_material, ai_texture_type, index, &path, NULL, NULL, NULL, NULL, NULL, NULL)
        != aiReturn_SUCCESS) {
        *err = Err_Assimp_Get_Texture;
        return;
    }

    usize const full_path_len = dir_path_str.len + 1 + path.length; // + 1 for the slash
    char *full_path = calloc(full_path_len + 1, sizeof(char));
    if (!full_path) {
        *err = Err_Calloc;
        return;
    }

    // @Note: we assume all texture paths are relative to dir_path_str.
    snprintf(full_path, full_path_len + 1, "%s" SLASH "%s", dir_path_str.data, &path.data[0]);

    // @Todo: first compute the material type and then, based on it, set apply_srgb_eotf.
    Texture texture = new_texture_from_filepath(full_path, SETTINGS_FOR_TEXTURE_STORE, err);
    texture.material_type =
        (ai_texture_type == aiTextureType_DIFFUSE    ? TextureMaterialType_Diffuse
         : ai_texture_type == aiTextureType_SPECULAR ? TextureMaterialType_Specular
         : ai_texture_type == aiTextureType_AMBIENT  ? TextureMaterialType_Ambient
         : ai_texture_type == aiTextureType_NORMALS  ? TextureMaterialType_Normal
         : ai_texture_type == aiTextureType_HEIGHT   ? TextureMaterialType_Height
                                                     : TextureMaterialType_None);

    // @Volatile: keep in sync with TextureMaterialType.
    if (texture.material_type == TextureMaterialType_None) {
        GLOW_WARNING("unhandled assimp aiTextureType: `%d`", ai_texture_type);
    }

    usize const len = texture_store->len;
    assert(len < texture_store->capacity);
    texture_store->len += 1;
    texture_store->paths[len] = alloc_str_copy(&path.data[0], err);
    texture_store->textures[len] = texture;

    if (*err) {
        GLOW_WARNING("failed to load texture from path: `%s`", full_path);
    } else {
        GLOW_LOG("Loaded texture: `%s`", full_path);
    }

    free(full_path);
}

static TextureStore create_texture_store_for_assimp_texture_types(
    Str const dir_path_str,
    struct aiScene const *ai_scene,
    enum aiTextureType const ai_texture_types[],
    usize ai_texture_types_len,
    Err *err) {
    if (*err) { return (TextureStore) { 0 }; }

    TextureStore texture_store = alloc_texture_store_for_assimp_texture_types(
        ai_scene, ai_texture_types, ai_texture_types_len, err);

    // Load material textures with the queried types.
    for (uint i = 0; i < ai_scene->mNumMaterials; ++i) {
        for (usize j = 0; j < ai_texture_types_len; ++j) {
            struct aiMaterial const *ai_material = ai_scene->mMaterials[i];
            enum aiTextureType const ai_texture_type = ai_texture_types[j];
            uint const count = aiGetMaterialTextureCount(ai_material, ai_texture_type);
            for (uint k = 0; k < count; ++k) {
                store_texture_with_assimp_material_texture_type_index(
                    &texture_store, dir_path_str, ai_material, ai_texture_type, k, err);
            }
        }
    }

    assert(texture_store.len == texture_store.capacity);
    return texture_store;
}

static void destroy_texture_store(TextureStore *texture_store) {
    dealloc_texture_store(texture_store);
}

static Texture *load_stored_texture_with_assimp_material_texture_type_index(
    TextureStore const *texture_store,
    struct aiMaterial const *ai_material,
    enum aiTextureType const ai_texture_type,
    uint index,
    Err *err) {
    if (*err) { return NULL; }

    struct aiString path = { 0 };
    if (aiGetMaterialTexture(
            ai_material, ai_texture_type, index, &path, NULL, NULL, NULL, NULL, NULL, NULL)
        != aiReturn_SUCCESS) {
        *err = Err_Assimp_Get_Texture;
        return NULL;
    }

    // Find the pre-loaded texture by comparing its path.
    for (usize i = 0; i < texture_store->len; ++i) {
        if (!strncmp(&path.data[0], texture_store->paths[i], path.length)) {
            return &texture_store->textures[i];
        }
    }

    GLOW_WARNING("could not find texture: `%s`", &path.data[0]);
    *err = Err_Model_Load_Stored_Texture;
    return NULL;
}

// @Cleanup: this isn't great... maybe it could be specified as an arg when creating the model?
static enum aiTextureType const STORED_ASSIMP_TEXTURE_TYPES[] = {
    aiTextureType_DIFFUSE, aiTextureType_SPECULAR, aiTextureType_AMBIENT,
    aiTextureType_NORMALS, aiTextureType_HEIGHT,
};

static Mesh alloc_mesh_from_assimp_mesh(
    TextureStore const *texture_store,
    struct aiScene const *ai_scene,
    struct aiMesh const *ai_mesh,
    Err *err) {
    if (*err) { return (Mesh) { 0 }; }

    // @Note: in assimp each mesh uses a single material (so because of this,
    // models that have multiple materials get split up into multiple meshes).
    struct aiMaterial *ai_material = ai_scene->mMaterials[ai_mesh->mMaterialIndex];
    GLOW_DEBUG("mMaterialIndex = %d", ai_mesh->mMaterialIndex);

    usize const textures_capacity = count_assimp_material_textures_with_types(
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

    for (usize j = 0; j < ARRAY_LEN(STORED_ASSIMP_TEXTURE_TYPES); ++j) {
        enum aiTextureType const ai_texture_type = STORED_ASSIMP_TEXTURE_TYPES[j];
        uint const count = aiGetMaterialTextureCount(ai_material, ai_texture_type);
        for (uint k = 0; k < count; ++k) {
            mesh.textures[mesh.textures_len++] =
                *load_stored_texture_with_assimp_material_texture_type_index(
                    texture_store, ai_material, ai_texture_type, k, err);
        }
    }
    assert(mesh.textures_len == textures_capacity);

    //
    // Mesh VAO.
    //

    mesh.vao = create_mesh_vao(mesh.vertices, mesh.vertices_len, mesh.indices, mesh.indices_len);

    return mesh;
}

static void alloc_into_model_meshes_from_assimp_node(
    Model *model,
    TextureStore const *texture_store,
    struct aiScene const *ai_scene,
    struct aiNode const *ai_node,
    Err *err) {
    if (*err) { return; }

    // Process ai_node's meshes.
    for (uint i = 0; i < ai_node->mNumMeshes; ++i) {
        model->meshes[model->meshes_len++] = alloc_mesh_from_assimp_mesh(
            texture_store, ai_scene, ai_scene->mMeshes[ai_node->mMeshes[i]], err);
        assert(model->meshes_len < model->meshes_capacity);
    }

    // Recursively process ai_node's children.
    for (uint i = 0; i < ai_node->mNumChildren; ++i) {
        alloc_into_model_meshes_from_assimp_node(
            model, texture_store, ai_scene, ai_node->mChildren[i], err);
    }
}

static Model
alloc_model_from_assimp_scene(char const *path, struct aiScene const *ai_scene, Err *err) {
    if (*err) { return (Model) { 0 }; }

    Model model = {
        .path = path,
        .meshes = calloc(ai_scene->mNumMeshes, sizeof(Mesh)),
        .meshes_len = 0,
        .meshes_capacity = ai_scene->mNumMeshes,
    };
    if (!model.meshes) { *err = Err_Calloc; }

    char *dir_path = alloc_str_copy(path, err);

    if (*err == Err_None) {
        terminate_at_last_path_component_inplace(dir_path);
        Str const dir_path_str = { dir_path, strlen(dir_path) };

        TextureStore texture_store = create_texture_store_for_assimp_texture_types(
            dir_path_str,
            ai_scene,
            STORED_ASSIMP_TEXTURE_TYPES,
            ARRAY_LEN(STORED_ASSIMP_TEXTURE_TYPES),
            err);

        // Recursively process nodes to convert assimp meshes.
        alloc_into_model_meshes_from_assimp_node(
            &model, &texture_store, ai_scene, ai_scene->mRootNode, err);
        assert(model.meshes_len == model.meshes_capacity);

        // Clean up the temporary mesh texture data allocated to create model.meshes.
        destroy_texture_store(&texture_store);
    }

    free(dir_path);

    return model;
}

#ifndef NDEBUG
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

    for (usize i = 0; i < model->meshes_len; ++i) {
        Mesh const *mesh = &model->meshes[i];
        count.indices += mesh->indices_len;
        count.vertices += mesh->vertices_len;
        count.textures += mesh->textures_len;
        for (usize j = 0; j < mesh->textures_len; ++j) {
            if (mesh->textures[j].material_type != TextureMaterialType_None) {
                count.materials += 1;
            }
        }
    }

    return count;
}
#endif

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

Model alloc_model_from_filepath_using_assimp(char const *path, Err *err) {
    struct aiScene const *ai_scene = aiImportFile(path, POST_PROCESS_FLAGS);

    if (!ai_scene || !ai_scene->mRootNode || (ai_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)) {
        GLOW_WARNING("assimp import failed with: `%s`", aiGetErrorString());
        *err = Err_Assimp_Import;
        return (Model) { 0 };
    }

    // @Todo: process materials.
    for (usize i = 0; i < ai_scene->mNumMaterials; ++i) {
        struct aiString name = { 0 };
        if (aiGetMaterialString(ai_scene->mMaterials[i], AI_MATKEY_NAME, &name)
            != aiReturn_SUCCESS) {
            assert(false);
        }
        GLOW_LOG("Material name: `%s`", name.data);
    }

    Model const model = alloc_model_from_assimp_scene(path, ai_scene, err);

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
