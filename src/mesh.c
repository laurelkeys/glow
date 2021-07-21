#include "mesh.h"

#include "console.h"
#include "maths.h"
#include "shader.h"
#include "texture.h"

#include <stdio.h>

#include <glad/glad.h>

uint init_mesh_vao(Vertex *vertices, usize vertices_len, uint *indices, usize indices_len) {
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
            2, 2, GL_FLOAT, GL_FALSE, stride, (void *) offsetof(Vertex, texcoord));

        glEnableVertexAttribArray(0); // vertex position
        glEnableVertexAttribArray(1); // vertex normal
        glEnableVertexAttribArray(2); // vertex texcoord
    }

    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vbo);

    return vao;
}

void dealloc_mesh(Mesh *mesh) {
    free(mesh->vertices);
    free(mesh->indices);
    free(mesh->textures);
    glDeleteVertexArrays(1, &mesh->vao);
}

#define MAX_SAMPLER_NAME_LEN 64

static void set_sampler_name_from_texture_material(
    char *name, usize max_len, TextureMaterialType const material, uint count) {
    int n = snprintf(name, max_len, sampler_name_from_texture_material(material));
    if (count) { snprintf(name + n, max_len, "%d", count); }
}

void draw_mesh_with_shader(Mesh const *mesh, Shader const *shader) {
    uint count[6] = { 0 }; // @Volatile: :SyncWithTextureMaterialType:

    char name[MAX_SAMPLER_NAME_LEN + 1] = { 0 };

    for (usize i = 0; i < mesh->textures_len; ++i) {
        TextureMaterialType const material = mesh->textures[i].material;
        assert((int) material <= ARRAY_LEN(count));
        assert((int) material > 0); // TextureMaterialType_None = 0

        // @Todo: store a char const *material_name in the Texture struct, and use string
        // interning for efficiency, since they will be immutable (and not always unique).
        set_sampler_name_from_texture_material(
            &name[0], MAX_SAMPLER_NAME_LEN, mesh->textures[i].material, count[material]);
        count[material] += 1;

        uint const texture_unit = GL_TEXTURE0 + (uint) i;
        set_shader_sampler2D(*shader, name, texture_unit);
        bind_texture_to_unit(mesh->textures[i], texture_unit);
    }

    glBindVertexArray(mesh->vao);
    DEFER(glBindVertexArray(0)) {
        // glDrawElements(GL_TRIANGLES, mesh->indices_len, GL_UNSIGNED_INT, 0);
        // @Temporary: just testing instancing without changing all function arguments...
        glDrawElementsInstanced(GL_TRIANGLES, mesh->indices_len, GL_UNSIGNED_INT, 0, 100);
    }
    bind_texture_to_unit((Texture) { 0 }, GL_TEXTURE0);
}

#undef MAX_SAMPLER_NAME_LEN
