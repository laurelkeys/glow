#pragma once

#include "prelude.h"

/* clang-format off */

typedef struct { f32 x, y; } vec2;
typedef struct { f32 x, y, z; } vec3;
typedef struct { f32 x, y, z, w; } vec4;

typedef struct { f32 m[3][3]; } mat3;
typedef struct { f32 m[4][4]; } mat4;

#if 0
typedef struct quat {
    f32 re; // scalar part (real)
    vec3 im; // vector part (imaginary)
} quat;
#endif
