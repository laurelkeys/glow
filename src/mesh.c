#include "mesh.h"

#include "console.h"
#include "maths.h"
#include "shader.h"
#include "texture.h"

#include <stdio.h>

#include <glad/glad.h>

uint create_mesh_vao(
    Vertex const *vertices, usize vertices_len, uint *indices, usize indices_len) {
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

        glEnableVertexAttribArray(0); // position
        glEnableVertexAttribArray(1); // normal
        glEnableVertexAttribArray(2); // texcoord

        glVertexAttribPointer(
            0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, position));
        glVertexAttribPointer(
            1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, normal));
        glVertexAttribPointer(
            2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, texcoord));
    }

    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vbo);

    return vao;
}

void destroy_mesh_vao(Mesh *mesh) {
    glDeleteVertexArrays(1, &mesh->vao);
}

void dealloc_mesh(Mesh *mesh) {
    // @Note: we are not calling glDeleteTextures.
    free(mesh->textures);
    mesh->textures = NULL;

    free(mesh->indices);
    mesh->indices = NULL;

    free(mesh->vertices);
    mesh->vertices = NULL;
}

void draw_mesh_direct(Mesh const *mesh) {
    glBindVertexArray(mesh->vao);
    DEFER(glBindVertexArray(0)) {
        glDrawElements(GL_TRIANGLES, mesh->indices_len, GL_UNSIGNED_INT, 0);
    }
}

static void set_sampler_name_from_texture_material_type(
    char *name, usize max_len, TextureMaterialType const material_type, uint count) {
    assert(material_type != TextureMaterialType_None);

    int const n = snprintf(
        name,
        max_len,
        // @Volatile: keep in sync with TextureMaterialType.
        (material_type == TextureMaterialType_Diffuse    ? "texture_diffuse"
         : material_type == TextureMaterialType_Specular ? "texture_specular"
         : material_type == TextureMaterialType_Ambient  ? "texture_ambient"
         : material_type == TextureMaterialType_Normal   ? "texture_normal"
         : material_type == TextureMaterialType_Height   ? "texture_height"
                                                         : "texture"));

    if (count > 0) { snprintf(name + n, max_len, "%d", count); }
}

void draw_mesh_with_shader(Mesh const *mesh, Shader const *shader) {
    uint count[6] = { 0 }; // @Volatile: keep in sync with TextureMaterialType.
    char name[24 + 1] = { 0 }; // @Note: large enough for all of sampler names.

    for (usize i = 0; i < mesh->textures_len; ++i) {
        TextureMaterialType const material_type = mesh->textures[i].material_type;
        assert((int) material_type <= (int) ARRAY_LEN(count));

        // @Todo: store a char const *material_name in the Texture struct, and use string
        // interning for efficiency, since they will be immutable (and not always unique).
        set_sampler_name_from_texture_material_type(
            &name[0], ARRAY_LEN(name), material_type, count[material_type]);

        count[material_type] += 1;

        uint const texture_unit = GL_TEXTURE0 + (uint) i;
        set_shader_sampler2D(*shader, name, texture_unit);
        bind_texture_to_unit(mesh->textures[i], texture_unit);
    }

    draw_mesh_direct(mesh);

    bind_texture_to_unit((Texture) { 0 }, GL_TEXTURE0);
}
