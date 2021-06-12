#pragma once

#include "prelude.h"

#include "maths.h"

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

typedef struct Mesh {
    Vertex *vertices; // @Ownership
    usize vertices_len;

    uint *indices; // @Ownership
    usize indices_len;

    Texture *textures; // @Ownership
    usize textures_len;

    uint vao;
} Mesh;

// @Note: this function assumes that the arrays of vertices, indices and textures
// have already been allocated, hence why it isn't called `alloc_new_mesh`.
// Despite that, it takes ownership over the given pointers, which is why we have
// `dealloc_mesh` (i.e. to clean them up).
Mesh new_mesh(
    Vertex *vertices,
    usize vertices_len,
    uint *indices,
    usize indices_len,
    Texture *textures,
    usize textures_len);
void dealloc_mesh(Mesh *mesh);

void draw_mesh_with_shader(Mesh const *mesh, Shader const *shader);
