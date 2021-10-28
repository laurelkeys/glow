#pragma once

#include "prelude.h"

typedef struct FullscreenQuad {
    uint vao;
} FullscreenQuad;

FullscreenQuad create_fullscreen_quad(void);
void destroy_fullscreen_quad(FullscreenQuad *quad);

void draw_fullscreen_quad(FullscreenQuad const *quad);
