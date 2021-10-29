#pragma once

#include "prelude.h"

#include "maths_types.h"

// Forward declarations.
typedef struct Shader Shader;
typedef struct Texture Texture;

typedef struct Vertex {
    vec3 position;
    vec3 normal;
    vec2 texcoord;
#if 0
    vec3 tangent;
    vec3 bitangent; // cross(normal, tangent)
#endif
} Vertex;

// @Speed: currently a Texture is no larger than two ints (it's an uint plus an enum),
// so it is cheap enough to copy. But if it ever gets larger, it'd be better to store
// texture handles inside of Mesh instead (i.e. usize indices into the model's array).
typedef struct Mesh {
    Vertex *vertices; // @Ownership
    usize vertices_len;

    uint *indices; // @Ownership
    usize indices_len;

    Texture *textures; // @Ownership
    usize textures_len;

    uint vao;
} Mesh;

uint create_mesh_vao(
    Vertex const *vertices, usize vertices_len, uint *indices, usize indices_len);
void destroy_mesh_vao(Mesh *mesh);

void dealloc_mesh(Mesh *mesh);

void draw_mesh_direct(Mesh const *mesh);
void draw_mesh_with_shader(Mesh const *mesh, Shader const *shader);
