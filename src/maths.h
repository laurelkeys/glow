#pragma once

#include "prelude.h"

#include <float.h>
#include <math.h>

//
// Constants.
//

#define M_PI 3.14159265358979323846
#define M_TAU 6.28318530717958647692

//
// Macro functions.
//

#define COMPARE(a, b) (((b) < (a)) - ((a) < (b)))
#define SIGN_OF(a) COMPARE((a), 0)

#define RADIANS(degrees) ((degrees) * (M_TAU / 360.0))
#define DEGREES(radians) ((radians) * (360.0 / M_TAU))

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define MIN3(a, b, c) (MIN(MIN((a), (b)), (c)))
#define MAX3(a, b, c) (MAX(MAX((a), (b)), (c)))
#define MIN4(a, b, c, d) (MIN(MIN((a), (b)), MIN((c), (d))))
#define MAX4(a, b, c, d) (MAX(MAX((a), (b)), MAX((c), (d))))

#define IS_ZERO(a) (((a) == 0.0) || (fabs(a) < FLT_EPSILON))
#define IS_EQ(a, b) (((a) == (b)) || (fabs((a) - (b)) < FLT_EPSILON))

#define CLAMP(x, a, b) (((x) < (a)) ? (a) : ((x) > (b)) ? (b) : (x))
#define CLAMP_MAX(x, b) (((x) > (b)) ? (b) : (x))
#define CLAMP_MIN(x, a) (((x) < (a)) ? (a) : (x))

//
// Scalar math.
//

f32 lerp(f32 a, f32 b, f32 t);
f32 clamp(f32 x, f32 x_min, f32 x_max);
f32 saturate(f32 x);

//
// Vector and matrix typedefs.
//

// clang-format off
typedef struct { f32 x, y; } vec2;
typedef struct { f32 x, y, z; } vec3;
typedef struct { f32 x, y, z, w; } vec4;

typedef struct { f32 m[3][3]; } mat3;
typedef struct { f32 m[4][4]; } mat4;
// clang-format on

//
// Common vector math.
//

#define COMMON_VECTOR_MATH_FOR(vec)      \
    vec vec##_of(f32 value);             \
    vec vec##_neg(vec v);                \
    vec vec##_min(vec a, vec b);         \
    vec vec##_max(vec a, vec b);         \
    vec vec##_add(vec a, vec b);         \
    vec vec##_sub(vec a, vec b);         \
    vec vec##_mul(vec a, vec b);         \
    vec vec##_scl(vec v, f32 factor);    \
    vec vec##_rcp(vec v);                \
    f32 vec##_dot(vec a, vec b);         \
    f32 vec##_length(vec v);             \
    vec vec##_normalize(vec v);          \
    vec vec##_lerp(vec a, vec b, f32 t); \
    vec vec##_saturate(vec v);

COMMON_VECTOR_MATH_FOR(vec2)
COMMON_VECTOR_MATH_FOR(vec3)
COMMON_VECTOR_MATH_FOR(vec4)

/*
vec2 vec2_map(vec2 v, f32 (*f)(f32));
vec3 vec3_map(vec3 v, f32 (*f)(f32));
vec4 vec4_map(vec4 v, f32 (*f)(f32));
vec2 vec2_zip(vec2 a, vec2 b, f32 (*f)(f32, f32));
vec3 vec3_zip(vec3 a, vec3 b, f32 (*f)(f32, f32));
vec4 vec4_zip(vec4 a, vec4 b, f32 (*f)(f32, f32));
*/

#undef COMMON_VECTOR_MATH_FOR

//
// 2D vector specific.
//

vec2 vec2_new(f32 x, f32 y);
vec2 vec2_from_vec3(vec3 v);
void vec2_print(char const *name, vec2 v);

//
// 3D vector specific.
//

vec3 vec3_new(f32 x, f32 y, f32 z);
vec3 vec3_from_vec2(vec2 v, f32 z);
vec3 vec3_from_vec4(vec4 v);
void vec3_print(char const *name, vec3 v);

vec3 vec3_cross(vec3 a, vec3 b);

//
// 4D vector specific.
//

vec4 vec4_new(f32 x, f32 y, f32 z, f32 w);
vec4 vec4_from_vec3(vec3 v, f32 w);
void vec4_print(char const *name, vec4 v);

//
// 3x3 matrix math.
//

mat3 mat3_id(void);
mat3 mat3_from_cols(vec3 c0, vec3 c1, vec3 c2);
mat3 mat3_from_mat4(mat4 m);
void mat3_print(char const *name, mat3 m);

mat3 mat3_scl(mat3 m, f32 factor);
vec3 mat3_mul_vec3(mat3 m, vec3 v);
mat3 mat3_mul_mat3(mat3 a, mat3 b);

mat3 mat3_inverse(mat3 m);
mat3 mat3_transpose(mat3 m);
mat3 mat3_inverse_transpose(mat3 m);

//
// 4x4 matrix math.
//

mat4 mat4_id(void);
void mat4_print(char const *name, mat4 m);

mat4 mat4_scl(mat4 m, f32 factor);
vec4 mat4_mul_vec4(mat4 m, vec4 v);
mat4 mat4_mul_mat4(mat4 a, mat4 b);

mat4 mat4_inverse(mat4 m);
mat4 mat4_transpose(mat4 m);
mat4 mat4_inverse_transpose(mat4 m);
