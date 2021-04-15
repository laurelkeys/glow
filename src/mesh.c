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
    {
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
    glBindVertexArray(0);

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
#define NAME_DIFFUSE "texture_diffuse"
#define NAME_SPECULAR "texture_specular"
#define MAX_NAME_LEN (MAX(sizeof(NAME_DIFFUSE "99"), sizeof(NAME_SPECULAR "99")))

void draw_mesh_with_shader(Mesh const *mesh, Shader const shader) {
    uint diffuse = 0;
    uint specular = 0;

    // @Robustness: could 100 textures not be enough?
    assert(mesh->textures_len <= 99);
    char name[MAX_NAME_LEN + 1] = { 0 };

    for (usize i = 0; i < mesh->textures_len; ++i) {
        switch (mesh->textures[i].type) {
            case TextureType_Diffuse:
                snprintf(name, MAX_NAME_LEN + 1, NAME_DIFFUSE "%d", ++diffuse);
                break;
            case TextureType_Specular:
                snprintf(name, MAX_NAME_LEN + 1, NAME_SPECULAR "%d", ++specular);
                break;
            // @Incomplete: what's the best way to handle TextureType_None?
            default:
                GLOW_WARNING(
                    "mesh texture with id `%d` has invalid type: `%d`",
                    mesh->textures[i].id,
                    mesh->textures[i].type);
                assert(false);
                return;
        }
        uint const texture_unit = GL_TEXTURE0 + (uint) i;
        set_shader_sampler2D(shader, name, texture_unit);
        bind_texture_to_unit(mesh->textures[i], texture_unit);
    }

    glBindVertexArray(mesh->vao);
    glDrawElements(GL_TRIANGLES, mesh->indices_len, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    bind_texture_to_unit((Texture) { 0 }, GL_TEXTURE0);
}

#undef MAX_NAME_LEN
#undef NAME_SPECULAR
#undef NAME_DIFFUSE
