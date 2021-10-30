#include "fullscreen_quad.h"

#include <glad/glad.h>

static f32 const QUAD_VERTICES_NDC[] = {
    -1, 1, 0, 0, 1, -1, -1, 0, 0, 0, 1, 1, 0, 1, 1, 1, -1, 0, 1, 0,
};

FullscreenQuad create_fullscreen_quad(void) {
    uint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    DEFER (glBindVertexArray(0)) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(
            GL_ARRAY_BUFFER, sizeof(QUAD_VERTICES_NDC), QUAD_VERTICES_NDC, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0); // position
        glEnableVertexAttribArray(1); // texcoord

        glVertexAttribPointer(
            0, 3, GL_FLOAT, GL_FALSE, sizeof(f32) * 5, (void *) (sizeof(f32) * 0));
        glVertexAttribPointer(
            1, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 5, (void *) (sizeof(f32) * 3));
    }

    glDeleteBuffers(1, &vbo);

    return (FullscreenQuad) { vao };
}

void destroy_fullscreen_quad(FullscreenQuad *quad) {
    glDeleteVertexArrays(1, &quad->vao);
}

void draw_fullscreen_quad(FullscreenQuad const *quad) {
    DEFER (glBindVertexArray(0)) {
        glBindVertexArray(quad->vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
}
