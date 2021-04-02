#pragma once

#include "prelude.h"

#include "maths.h"
#include "shader.h"
#include "texture.h"

typedef enum MeshTextureType {
    MeshTextureType_Diffuse,
    MeshTextureType_Specular,
} MeshTextureType;

typedef struct MeshTexture {
    MeshTextureType type;
    char const *path;
    Texture it;
} MeshTexture;

typedef struct MeshVertex {
    vec3 position;
    vec3 normal;
    vec2 texcoords;
#if 0
    vec3 tangent;
    vec3 bitangent; // cross(normal, tangent)
#endif
} MeshVertex;

typedef struct Mesh {
    MeshVertex const *vertices;
    usize vertices_len;

    uint const *indices;
    usize indices_len;

    MeshTexture const *textures;
    usize textures_len;

    uint vao;
} Mesh;

// @Note: Mesh doesn't take "ownership" over any pointers.
Mesh new_mesh(
    MeshVertex const *vertices,
    usize vertices_len,
    uint const *indices,
    usize indices_len,
    MeshTexture const *textures,
    usize textures_len);

void draw_mesh_with_shader(Mesh const *mesh, Shader const shader);
