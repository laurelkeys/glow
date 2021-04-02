#include "mesh.h"

#include "console.h"
#include "opengl.h"

#include <stdio.h>

Mesh new_mesh(
    MeshVertex const *vertices,
    usize vertices_len,
    uint const *indices,
    usize indices_len,
    MeshTexture const *textures,
    usize textures_len) {
    uint vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(
            GL_ARRAY_BUFFER, sizeof(MeshVertex) * vertices_len, &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indices_len, &indices[0], GL_STATIC_DRAW);

        uint const stride = sizeof(MeshVertex);
        glVertexAttribPointer(
            0, 3, GL_FLOAT, GL_FALSE, stride, (void *) offsetof(MeshVertex, position));
        glVertexAttribPointer(
            1, 3, GL_FLOAT, GL_FALSE, stride, (void *) offsetof(MeshVertex, normal));
        glVertexAttribPointer(
            2, 2, GL_FLOAT, GL_FALSE, stride, (void *) offsetof(MeshVertex, texcoords));

        glEnableVertexAttribArray(0); // vertex position
        glEnableVertexAttribArray(1); // vertex normal
        glEnableVertexAttribArray(2); // vertex texcoords
    }
    glBindVertexArray(0);

    return (Mesh) {
        vertices, vertices_len, indices, indices_len, textures, textures_len, vao, vbo, ebo,
    };
}

#define NAME_DIFFUSE "material.diffuse"
#define NAME_SPECULAR "material.specular"
#define MAX_NAME_LEN (MAX(sizeof(NAME_DIFFUSE), sizeof(NAME_SPECULAR)) + sizeof("99"))

void draw_mesh_with_shader(Mesh const *mesh, Shader const shader) {
    uint diffuse = 0;
    uint specular = 0;

    assert(mesh->textures_len <= 99);
    char name[MAX_NAME_LEN + 1];

    for (int i = 0; i < mesh->textures_len; ++i) {
        switch (mesh->textures[i].type) {
            case MeshTextureType_Diffuse:
                snprintf(name, MAX_NAME_LEN, NAME_DIFFUSE "%d", ++diffuse);
                break;
            case MeshTextureType_Specular:
                snprintf(name, MAX_NAME_LEN, NAME_SPECULAR "%d", ++specular);
                break;
            default:
                GLOW_WARNING(
                    "mesh texture with id `%d` has invalid type: `%d`",
                    mesh->textures[i].it.id,
                    mesh->textures[i].type);
                assert(false);
                return;
        }
        uint const texture_unit = GL_TEXTURE0 + (uint) i;
        set_shader_sampler2D(shader, name, texture_unit);
        bind_texture_to_unit(mesh->textures[i].it, texture_unit);
    }

    glBindVertexArray(mesh->vao);
    glDrawElements(GL_TRIANGLES, mesh->indices_len, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

#undef MAX_NAME_LEN
#undef NAME_SPECULAR
#undef NAME_DIFFUSE
