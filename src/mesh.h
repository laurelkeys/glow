#pragma once

#include "prelude.h"

#include "maths.h"
#include "shader.h"
#include "texture.h"

typedef struct Vertex {
    vec3 position;
    vec3 normal;
    vec2 texcoords;
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

Mesh new_mesh(
    Vertex *vertices,
    usize vertices_len,
    uint *indices,
    usize indices_len,
    Texture *textures,
    usize textures_len);
void dealloc_mesh(Mesh *mesh);

void draw_mesh_with_shader(Mesh const *mesh, Shader const shader);
