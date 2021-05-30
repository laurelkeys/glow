#include "mesh.h"

#include "console.h"
#include "opengl.h"

#include <stdio.h>

static uint
init_mesh_vao(Vertex *vertices, usize vertices_len, uint *indices, usize indices_len) {
    uint vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
    DEFER(glBindVertexArray(0)) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(
            GL_ARRAY_BUFFER, sizeof(Vertex) * vertices_len, &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indices_len, &indices[0], GL_STATIC_DRAW);

        uint const stride = sizeof(Vertex);
        glVertexAttribPointer(
            0, 3, GL_FLOAT, GL_FALSE, stride, (void *) offsetof(Vertex, position));
        glVertexAttribPointer(
            1, 3, GL_FLOAT, GL_FALSE, stride, (void *) offsetof(Vertex, normal));
        glVertexAttribPointer(
            2, 2, GL_FLOAT, GL_FALSE, stride, (void *) offsetof(Vertex, texcoords));

        glEnableVertexAttribArray(0); // vertex position
        glEnableVertexAttribArray(1); // vertex normal
        glEnableVertexAttribArray(2); // vertex texcoords
    }

    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vbo);

    return vao;
}

Mesh new_mesh(
    Vertex *vertices,
    usize vertices_len,
    uint *indices,
    usize indices_len,
    Texture *textures,
    usize textures_len) {
    return (Mesh) {
        vertices,
        vertices_len,

        indices,
        indices_len,

        textures,
        textures_len,

        init_mesh_vao(vertices, vertices_len, indices, indices_len),
    };
}

void dealloc_mesh(Mesh *mesh) {
    free(mesh->vertices);
    free(mesh->indices);
    free(mesh->textures);
    glDeleteVertexArrays(1, &mesh->vao);
}

// @Fixme: stop using hard-coded names.
#define NAME_AMBIENT "material.ambient"
#define NAME_DIFFUSE "material.diffuse"
#define NAME_SPECULAR "material.specular"
#define MAX_NAME_LEN \
    (MAX3(sizeof(NAME_AMBIENT "99"), sizeof(NAME_DIFFUSE "99"), sizeof(NAME_SPECULAR "99")))
#define SET_NAME(name, NAME_TYPE, type_count)                           \
    if (type_count++ == 0) {                                            \
        snprintf((name), MAX_NAME_LEN + 1, NAME_TYPE);                  \
    } else {                                                            \
        snprintf((name), MAX_NAME_LEN + 1, NAME_TYPE "%d", type_count); \
    }

void draw_mesh_with_shader(Mesh const *mesh, Shader const shader) {
    uint ambient = 0;
    uint diffuse = 0;
    uint specular = 0;

    assert(mesh->textures_len <= 99);
    char name[MAX_NAME_LEN + 1] = { 0 };

    for (usize i = 0; i < mesh->textures_len; ++i) {
        // @Todo: store a char const *material_name in the Texture struct, and use string
        // interning for efficiency, since they will be immutable (and not always unique).
        switch (mesh->textures[i].material) {
            case TextureMaterialType_Ambient: SET_NAME(name, NAME_AMBIENT, ambient); break;
            case TextureMaterialType_Diffuse: SET_NAME(name, NAME_DIFFUSE, diffuse); break;
            case TextureMaterialType_Specular: SET_NAME(name, NAME_SPECULAR, specular); break;
            // @Incomplete: what's the best way to handle TextureMaterialType_None?
            default:
                GLOW_WARNING(
                    "mesh texture with id `%d` has invalid material type: `%d`",
                    mesh->textures[i].id,
                    mesh->textures[i].material);
                assert(false);
                return;
        }
        uint const texture_unit = GL_TEXTURE0 + (uint) i;
        set_shader_sampler2D(shader, name, texture_unit);
        bind_texture_to_unit(mesh->textures[i], texture_unit);
    }

    glBindVertexArray(mesh->vao);
    DEFER(glBindVertexArray(0)) {
        glDrawElements(GL_TRIANGLES, mesh->indices_len, GL_UNSIGNED_INT, 0);
    }
    bind_texture_to_unit((Texture) { 0 }, GL_TEXTURE0);
}

#undef SET_NAME
#undef MAX_NAME_LEN
#undef NAME_SPECULAR
#undef NAME_DIFFUSE
