#include "mesh.h"

#include "console.h"
#include "maths.h"
#include "shader.h"
#include "texture.h"

#include <stdio.h>

#include <glad/glad.h>

uint make_mesh_vao(Vertex const *vertices, usize vertices_len, uint *indices, usize indices_len) {
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

void dealloc_mesh(Mesh *mesh) {
    // @Note: we are not calling glDeleteTextures.
    glDeleteVertexArrays(1, &mesh->vao);

    free(mesh->textures);
    mesh->textures = NULL;

    free(mesh->indices);
    mesh->indices = NULL;

    free(mesh->vertices);
    mesh->vertices = NULL;
}

// @Volatile: :SyncWithTextureMaterialType:
static char const *SAMPLER_NAME_FROM_MATERIAL_TYPE[] = {
    [TextureMaterialType_Diffuse] = "texture_diffuse",
    [TextureMaterialType_Specular] = "texture_specular",
    [TextureMaterialType_Ambient] = "texture_ambient",
    [TextureMaterialType_Normal] = "texture_normal",
    [TextureMaterialType_Height] = "texture_height",
};

// @Volatile: :SyncWithTextureMaterialType:
STATIC_ASSERT(ARRAY_LEN(SAMPLER_NAME_FROM_MATERIAL_TYPE) == 6);

static void set_sampler_name_from_texture_material(
    char *name, usize max_len, TextureMaterialType const material, uint count) {
    int n = snprintf(name, max_len, SAMPLER_NAME_FROM_MATERIAL_TYPE[material]);
    if (count) { snprintf(name + n, max_len, "%d", count); }
}

void draw_mesh_with_shader(Mesh const *mesh, Shader const *shader) {
    uint count[ARRAY_LEN(SAMPLER_NAME_FROM_MATERIAL_TYPE)] = { 0 };
    char name[24 + 1] = { 0 }; // @Note: large enough for SAMPLER_NAME_FROM_MATERIAL_TYPE

    for (usize i = 0; i < mesh->textures_len; ++i) {
        TextureMaterialType const material = mesh->textures[i].material;
        assert((int) material <= (int) ARRAY_LEN(count));
        assert((int) material > 0); // 0 = TextureMaterialType_None

        // @Todo: store a char const *material_name in the Texture struct, and use string
        // interning for efficiency, since they will be immutable (and not always unique).
        set_sampler_name_from_texture_material(
            &name[0], ARRAY_LEN(name), mesh->textures[i].material, count[material]);

        count[material] += 1;

        uint const texture_unit = GL_TEXTURE0 + (uint) i;
        set_shader_sampler2D(*shader, name, texture_unit);
        bind_texture_to_unit(mesh->textures[i], texture_unit);
    }

    glBindVertexArray(mesh->vao);
    DEFER(glBindVertexArray(0)) {
        glDrawElements(GL_TRIANGLES, mesh->indices_len, GL_UNSIGNED_INT, 0);
    }

    bind_texture_to_unit((Texture) { 0 }, GL_TEXTURE0);
}
