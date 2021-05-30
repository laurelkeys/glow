#pragma once

#include "prelude.h"

#include "maths.h"

// clang-format off
static f32 const QUAD_VERTICES[] = {
    // positions     // texture coords
    -1.0f,  1.0f,    0.0f, 1.0f,
    -1.0f, -1.0f,    0.0f, 0.0f,
     1.0f, -1.0f,    1.0f, 0.0f,

    -1.0f,  1.0f,    0.0f, 1.0f,
     1.0f, -1.0f,    1.0f, 0.0f,
     1.0f,  1.0f,    1.0f, 1.0f
};

static f32 const SKYBOX_VERTICES[] = {
    // positions
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
};

static f32 const CUBE_VERTICES[] = {
    // positions            // normals             // texture coords
    -0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,    0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,    1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,    1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,    1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,    0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,    0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,    0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,    1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,    1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,    1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,    0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,    0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,    1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,    1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,    0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,    0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,    0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,    1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,    1.0f,  0.0f,  0.0f,    1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,    1.0f,  0.0f,  0.0f,    1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,    1.0f,  0.0f,  0.0f,    0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,    1.0f,  0.0f,  0.0f,    0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,    1.0f,  0.0f,  0.0f,    0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,    1.0f,  0.0f,  0.0f,    1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,    0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,    1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,    1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,    1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,    0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,    0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,    0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,    1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,    1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,    1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,    0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,    0.0f, 1.0f,
};

static f32 const FLOOR_VERTICES[] = {
    // positions            // texture coords
     5.0f, -0.5f,  5.0f,    2.0f, 0.0f,
    -5.0f, -0.5f,  5.0f,    0.0f, 0.0f,
    -5.0f, -0.5f, -5.0f,    0.0f, 2.0f,

     5.0f, -0.5f,  5.0f,    2.0f, 0.0f,
    -5.0f, -0.5f, -5.0f,    0.0f, 2.0f,
     5.0f, -0.5f, -5.0f,    2.0f, 2.0f,
};

static f32 const TRANSPARENT_VERTICES[] = {
    // positions            // texture coords
     0.0f,  0.5f,  0.0f,    1.0f, 1.0f,
     0.0f, -0.5f,  0.0f,    1.0f, 0.0f,
     1.0f, -0.5f,  0.0f,    0.0f, 0.0f,

     0.0f,  0.5f,  0.0f,    1.0f, 1.0f,
     1.0f, -0.5f,  0.0f,    0.0f, 0.0f,
     1.0f,  0.5f,  0.0f,    0.0f, 1.0f,
};
// clang-format on

#if 0
typedef struct VertexLocations {
    int position;
    int normal;
    int texcoords;
} VertexLocations;

static VertexLocations const Default_VertexLocations = { .position = 0, .normal = 1, .texcoords = 2 };

#include "mesh.h"

static Vertex const CUBE_VERTICES[] = {
    { .position = { -0.5f, -0.5f, -0.5f }, .normal = { 0, 0, -1 }, .texcoords = { 0, 0 } },
    { .position = { +0.5f, -0.5f, -0.5f }, .normal = { 0, 0, -1 }, .texcoords = { 1, 0 } },
    { .position = { +0.5f, +0.5f, -0.5f }, .normal = { 0, 0, -1 }, .texcoords = { 1, 1 } },
    { .position = { +0.5f, +0.5f, -0.5f }, .normal = { 0, 0, -1 }, .texcoords = { 1, 1 } },
    { .position = { -0.5f, +0.5f, -0.5f }, .normal = { 0, 0, -1 }, .texcoords = { 0, 1 } },
    { .position = { -0.5f, -0.5f, -0.5f }, .normal = { 0, 0, -1 }, .texcoords = { 0, 0 } },

    { .position = { -0.5f, -0.5f, +0.5f }, .normal = { 0, 0, +1 }, .texcoords = { 0, 0 } },
    { .position = { +0.5f, -0.5f, +0.5f }, .normal = { 0, 0, +1 }, .texcoords = { 1, 0 } },
    { .position = { +0.5f, +0.5f, +0.5f }, .normal = { 0, 0, +1 }, .texcoords = { 1, 1 } },
    { .position = { +0.5f, +0.5f, +0.5f }, .normal = { 0, 0, +1 }, .texcoords = { 1, 1 } },
    { .position = { -0.5f, +0.5f, +0.5f }, .normal = { 0, 0, +1 }, .texcoords = { 0, 1 } },
    { .position = { -0.5f, -0.5f, +0.5f }, .normal = { 0, 0, +1 }, .texcoords = { 0, 0 } },

    { .position = { -0.5f, +0.5f, +0.5f }, .normal = { -1, 0, 0 }, .texcoords = { 1, 0 } },
    { .position = { -0.5f, +0.5f, -0.5f }, .normal = { -1, 0, 0 }, .texcoords = { 1, 1 } },
    { .position = { -0.5f, -0.5f, -0.5f }, .normal = { -1, 0, 0 }, .texcoords = { 0, 1 } },
    { .position = { -0.5f, -0.5f, -0.5f }, .normal = { -1, 0, 0 }, .texcoords = { 0, 1 } },
    { .position = { -0.5f, -0.5f, +0.5f }, .normal = { -1, 0, 0 }, .texcoords = { 0, 0 } },
    { .position = { -0.5f, +0.5f, +0.5f }, .normal = { -1, 0, 0 }, .texcoords = { 1, 0 } },

    { .position = { +0.5f, +0.5f, +0.5f }, .normal = { +1, 0, 0 }, .texcoords = { 1, 0 } },
    { .position = { +0.5f, +0.5f, -0.5f }, .normal = { +1, 0, 0 }, .texcoords = { 1, 1 } },
    { .position = { +0.5f, -0.5f, -0.5f }, .normal = { +1, 0, 0 }, .texcoords = { 0, 1 } },
    { .position = { +0.5f, -0.5f, -0.5f }, .normal = { +1, 0, 0 }, .texcoords = { 0, 1 } },
    { .position = { +0.5f, -0.5f, +0.5f }, .normal = { +1, 0, 0 }, .texcoords = { 0, 0 } },
    { .position = { +0.5f, +0.5f, +0.5f }, .normal = { +1, 0, 0 }, .texcoords = { 1, 0 } },

    { .position = { -0.5f, -0.5f, -0.5f }, .normal = { 0, +1, 0 }, .texcoords = { 0, 1 } },
    { .position = { +0.5f, -0.5f, -0.5f }, .normal = { 0, +1, 0 }, .texcoords = { 1, 1 } },
    { .position = { +0.5f, -0.5f, +0.5f }, .normal = { 0, +1, 0 }, .texcoords = { 1, 0 } },
    { .position = { +0.5f, -0.5f, +0.5f }, .normal = { 0, +1, 0 }, .texcoords = { 1, 0 } },
    { .position = { -0.5f, -0.5f, +0.5f }, .normal = { 0, +1, 0 }, .texcoords = { 0, 0 } },
    { .position = { -0.5f, -0.5f, -0.5f }, .normal = { 0, +1, 0 }, .texcoords = { 0, 1 } },

    { .position = { -0.5f, +0.5f, -0.5f }, .normal = { 0, +1, 0 }, .texcoords = { 0, 1 } },
    { .position = { +0.5f, +0.5f, -0.5f }, .normal = { 0, +1, 0 }, .texcoords = { 1, 1 } },
    { .position = { +0.5f, +0.5f, +0.5f }, .normal = { 0, +1, 0 }, .texcoords = { 1, 0 } },
    { .position = { +0.5f, +0.5f, +0.5f }, .normal = { 0, +1, 0 }, .texcoords = { 1, 0 } },
    { .position = { -0.5f, +0.5f, +0.5f }, .normal = { 0, +1, 0 }, .texcoords = { 0, 0 } },
    { .position = { -0.5f, +0.5f, -0.5f }, .normal = { 0, +1, 0 }, .texcoords = { 0, 1 } },
};

static Vertex const QUAD_VERTICES[] = {
    { .position = { -1.0f, +1.0f }, .texcoords = { 0, 1 } },
    { .position = { -1.0f, -1.0f }, .texcoords = { 0, 0 } },
    { .position = { +1.0f, -1.0f }, .texcoords = { 1, 0 } },

    { .position = { -1.0f, +1.0f }, .texcoords = { 0, 1 } },
    { .position = { +1.0f, -1.0f }, .texcoords = { 1, 0 } },
    { .position = { +1.0f, +1.0f }, .texcoords = { 1, 1 } },
};

/*
static Vertex const SKYBOX_VERTICES[] = {
    { -1, 1, -1 },  { -1, -1, -1 }, { 1, -1, -1 }, { 1, -1, -1 }, { 1, 1, -1 },  { -1, 1, -1 },
    { -1, -1, 1 },  { -1, -1, -1 }, { -1, 1, -1 }, { -1, 1, -1 }, { -1, 1, 1 },  { -1, -1, 1 },
    { 1, -1, -1 },  { 1, -1, 1 },   { 1, 1, 1 },   { 1, 1, 1 },   { 1, 1, -1 },  { 1, -1, -1 },
    { -1, -1, 1 },  { -1, 1, 1 },   { 1, 1, 1 },   { 1, 1, 1 },   { 1, -1, 1 },  { -1, -1, 1 },
    { -1, 1, -1 },  { 1, 1, -1 },   { 1, 1, 1 },   { 1, 1, 1 },   { -1, 1, 1 },  { -1, 1, -1 },
    { -1, -1, -1 }, { -1, -1, 1 },  { 1, -1, -1 }, { 1, -1, -1 }, { -1, -1, 1 }, { 1, -1, 1 },
};
*/

static Vertex const FLOOR_VERTICES[] = {
    { .position = { +5.0f, -0.5f, +5.0f }, .texcoords = { 2, 0 } },
    { .position = { -5.0f, -0.5f, +5.0f }, .texcoords = { 0, 0 } },
    { .position = { -5.0f, -0.5f, -5.0f }, .texcoords = { 0, 2 } },

    { .position = { +5.0f, -0.5f, +5.0f }, .texcoords = { 2, 0 } },
    { .position = { -5.0f, -0.5f, -5.0f }, .texcoords = { 0, 2 } },
    { .position = { +5.0f, -0.5f, -5.0f }, .texcoords = { 2, 2 } },
};

static Vertex const TRANSPARENT_VERTICES[] = {
    { .position = { 0.0f, +0.5f, 0.0f }, .texcoords = { 1, 1 } },
    { .position = { 0.0f, -0.5f, 0.0f }, .texcoords = { 1, 0 } },
    { .position = { 1.0f, -0.5f, 0.0f }, .texcoords = { 0, 0 } },

    { .position = { 0.0f, +0.5f, 0.0f }, .texcoords = { 1, 1 } },
    { .position = { 1.0f, -0.5f, 0.0f }, .texcoords = { 0, 0 } },
    { .position = { 1.0f, +0.5f, 0.0f }, .texcoords = { 0, 1 } },
};
#endif
