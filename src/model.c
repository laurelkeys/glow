#include "model.h"

#include "console.h"
#include "file.h"

#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

MeshTextureType mesh_texture_type_from_assimp(enum aiTextureType type) {
    switch (type) {
        default:
            GLOW_WARNING("unkown MeshTextureType equivalent to assimp aiTextureType: `%d`", type);
            assert(false); // fallthrough in release mode
        case aiTextureType_DIFFUSE: return MeshTextureType_Diffuse;
        case aiTextureType_SPECULAR: return MeshTextureType_Specular;
    }
}

// @Speed: we could (store and) reuse textures that have already been loaded.
void load_material_textures(
    Model const *model,
    MeshTexture *textures,
    uint start,
    uint end,
    struct aiMaterial const *material,
    enum aiTextureType type,
    Err *err) {
    usize const dir_path_len = strlen(model->dir_path);

    struct aiString *path = NULL;

    for (usize i = start; i < end; ++i) {
        if (aiGetMaterialTexture(material, type, i, path, NULL, NULL, NULL, NULL, NULL, NULL)
            != aiReturn_SUCCESS) {
            *err = Err_Assimp_Get_Texture;
            return;
        }

        // @Note: we assume all texture paths are relative, and prefix them with dir_path.
        usize const full_path_len = dir_path_len + strlen(path->data);
        char *full_path = calloc(full_path_len + 1, sizeof(char));
        sprintf(full_path, "%s%s", model->dir_path, path->data); // @Fixme: might need a '/'

        GLOW_DEBUG("[load_material_textures] i = %d, path = '%s'", i, path->data);
        GLOW_DEBUG("[load_material_textures] full_path    = '%s'", full_path);
        textures[i] = (MeshTexture) {
            .type = mesh_texture_type_from_assimp(type),
            .path = path->data, // @Leak: maybe? Do we need to copy it?
            .it = new_texture_from_filepath(path->data, err), // @Fixme: use full_path
        };
    }
}

static Mesh process_mesh(
    Model const *model, struct aiMesh const *mesh, struct aiScene const *scene, Err *err) {
    //
    // Mesh vertices.
    //

    MeshVertex *vertices = calloc(mesh->mNumVertices, sizeof(MeshVertex));
    if (!vertices) { return (*err = Err_Calloc, (Mesh) { 0 }); }

    for (uint i = 0; i < mesh->mNumVertices; ++i) {
        struct aiVector3D position = mesh->mVertices[i];
        struct aiVector3D normal = mesh->mNormals[i];
        struct aiVector3D texcoords = (mesh->mTextureCoords[0] != NULL)
                                          ? mesh->mTextureCoords[0][i]
                                          : (struct aiVector3D) { 0 };
        vertices[i] = (MeshVertex) {
            { position.x, position.y, position.z },
            { normal.x, normal.y, normal.z },
            { texcoords.x, texcoords.y },
        };
    }

    //
    // Mesh indices.
    //

    // @Volatile: all faces are assumed to be triangular due to aiProcess_Triangulate.
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

    usize const textures_len = diffuse_len + specular_len;
    MeshTexture *textures = calloc(textures_len, sizeof(MeshTexture));
    if (!textures) {
        free(vertices);
        free(indices);
        return (*err = Err_Calloc, (Mesh) { 0 });
    }

#define LOAD_MATERIAL_TEXTURES(type, start, end) \
    load_material_textures(model, textures, start, end, material, type, err)

    LOAD_MATERIAL_TEXTURES(aiTextureType_DIFFUSE, 0, diffuse_len);
    LOAD_MATERIAL_TEXTURES(aiTextureType_SPECULAR, diffuse_len, textures_len);

#undef LOAD_MATERIAL_TEXTURES

    return new_mesh(
        vertices, mesh->mNumVertices, indices, 3 * mesh->mNumFaces, textures, textures_len);
}

static bool
process_node(Model *model, struct aiNode const *node, struct aiScene const *scene, Err *err) {
    if (node->mNumMeshes == 0) {
        assert(node->mNumChildren == 0);
        return true;
    }

    GLOW_DEBUG("[process_node] model->meshes_len = %d", model->meshes_len);
    GLOW_DEBUG("[process_node] node->mNumMeshes  = %d", node->mNumMeshes);
    model->meshes = realloc(model->meshes, sizeof(Mesh) * (model->meshes_len + node->mNumMeshes));
    if (!model->meshes) { return (*err = Err_Realloc, false); }

    for (uint i = 0; i < node->mNumMeshes; ++i) {
        struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        model->meshes[(model->meshes_len)++] = process_mesh(model, mesh, scene, err);
    }

    if (*err) { return false; }

    for (uint i = 0; i < node->mNumChildren; ++i) {
        process_node(model, node->mChildren[i], scene, err);
    }

    return *err == Err_None;
}

Model alloc_new_model_from_filepath(char const *model_path, Err *err) {
    uint const post_process_flags =
        aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_ValidateDataStructure
        /* | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace */;

    struct aiScene const *scene =
        aiImportFile(model_path, post_process_flags); // @Volatile: see `process_mesh`

    if (!scene || !scene->mRootNode || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)) {
        GLOW_WARNING("assimp import failed with: `%s`", aiGetErrorString());
        return (*err = Err_Assimp_Import, (Model) { 0 });
    }

    char *dir_path = alloc_str_copy(model_path);
    terminate_at_last_path_component(dir_path);
    GLOW_DEBUG("[alloc_new_model_from_filepath] model_path = '%s'", model_path);
    GLOW_DEBUG("[alloc_new_model_from_filepath] dir_path   = '%s'", dir_path);

    Model model = { dir_path, calloc(0, sizeof(Mesh)), 0 };
    process_node(&model, scene->mRootNode, scene, err);
    aiReleaseImport(scene);

    return model;
}

void draw_model_with_shader(Model const *model, Shader const shader) {
    for (usize i = 0; i < model->meshes_len; ++i) {
        draw_mesh_with_shader(&model->meshes[i], shader);
    }
}
