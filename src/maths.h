#pragma once

#include "prelude.h"

#include <float.h>
#include <math.h>

//
// Typedefs.
//

#include "maths_types.h"

//
// Constants.
//

#ifndef M_PI
#define M_PI 3.1415926535897932
#endif

#define M_TAU 6.2831853071795865
#define M_SQRT_2 1.4142135623730950
#define M_SQRT_3 1.7320508075688773

#define M_E 2.7182818284590452
#define M_LN_2 0.6931471805599453
#define M_LN_10 2.3025850929940457
#define M_LOG2_E 1.4426950408889634
#define M_LOG10_E 0.4342944819032518

#define M_PI_DIV_2 1.5707963267948966
#define M_PI_DIV_4 0.7853981633974483
#define M_1_DIV_PI 0.3183098861837907
#define M_2_DIV_PI 0.6366197723675813
#define M_1_DIV_SQRT_2 0.7071067811865475

//
// Macro functions.
//

#define COMPARE(a, b) (((b) < (a)) - ((a) < (b)))
#define SIGN_OF(a) COMPARE((a), 0)

#define RADIANS_FROM_DEGREES(degrees) ((degrees) * (M_TAU / 360.0))
#define DEGREES_FROM_RADIANS(radians) ((radians) * (360.0 / M_TAU))

#define ABS(a) ((a) < 0 ? -(a) : (a))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) < (b) ? (b) : (a))

#define MIN3(a, b, c) (MIN(MIN((a), (b)), (c)))
#define MAX3(a, b, c) (MAX(MAX((a), (b)), (c)))
#define MIN4(a, b, c, d) (MIN(MIN((a), (b)), MIN((c), (d))))
#define MAX4(a, b, c, d) (MAX(MAX((a), (b)), MAX((c), (d))))

#define APPROX_EQ(a, b, eps) (((a) == (b)) || (fabs((a) - (b)) < (eps)))
#define APPROX_ZERO(a, eps) APPROX_EQ((a), 0.0, (eps))
#define IS_EQ(a, b) APPROX_EQ((a), (b), FLT_EPSILON)
#define IS_ZERO(a) IS_EQ((a), 0.0, FLT_EPSILON)

#define STEP(edge, x) (((x) < (edge)) ? 0 : 1)

#define CLAMP(x, a, b) (((x) < (a)) ? (a) : ((x) > (b)) ? (b) : (x))
#define CLAMP_MIN(x, a) (((x) < (a)) ? (a) : (x))
#define CLAMP_MAX(x, b) (((x) > (b)) ? (b) : (x))

#define RANDOM(a, b) ((a) + ((b) - (a)) * ((f32) rand() / (f32) RAND_MAX))

#define DIV_CEIL(dividend, divisor) ((dividend) / (divisor) + ((dividend) % (divisor) ? 1 : 0))

//
// Scalar math.
//

f32 fract(f32 x);

f32 lerp(f32 a, f32 b, f32 t);
f32 unlerp(f32 a, f32 b, f32 value); // inverse lerp
f32 remap(f32 a0, f32 b0, f32 a1, f32 b1, f32 value);

f32 clamp(f32 x, f32 x_min, f32 x_max);
f32 saturate(f32 x);

f32 move_toward(f32 a, f32 b, f32 amount);

//
// Common vector math.
//

#define COMMON_VECTOR_MATH_FOR(vec)                  \
    vec vec##_of(f32 value);                         \
    vec vec##_neg(vec const v);                      \
    vec vec##_min(vec const a, vec const b);         \
    vec vec##_max(vec const a, vec const b);         \
    vec vec##_add(vec const a, vec const b);         \
    vec vec##_sub(vec const a, vec const b);         \
    vec vec##_mul(vec const a, vec const b);         \
    vec vec##_scl(vec const v, f32 factor);          \
    vec vec##_rcp(vec const v);                      \
    f32 vec##_dot(vec const a, vec const b);         \
    f32 vec##_length(vec const v);                   \
    vec vec##_normalize(vec const v);                \
    vec vec##_lerp(vec const a, vec const b, f32 t); \
    vec vec##_saturate(vec const v);

COMMON_VECTOR_MATH_FOR(vec2)
COMMON_VECTOR_MATH_FOR(vec3)
COMMON_VECTOR_MATH_FOR(vec4)

#undef COMMON_VECTOR_MATH_FOR

//
// 2D vector specific.
//

void vec2_print(char const *name, vec2 const v);
vec2 vec2_from_vec3(vec3 const v);

vec2 vec2_rot_90cw(vec2 const v);
vec2 vec2_rot_90ccw(vec2 const v);
f32 vec2_perp_dot(vec2 const a, vec2 const b);

//
// 3D vector specific.
//

void vec3_print(char const *name, vec3 const v);
vec3 vec3_from_vec2(vec2 const v, f32 z);
vec3 vec3_from_vec4(vec4 const v);

vec3 vec3_cross(vec3 const a, vec3 const b);

#if 0
f32 scalar_triple_product(vec3 const a, vec3 const b, vec3 const c); // a . (b x c)
vec3 vector_triple_product(vec3 const a, vec3 const b, vec3 const c); // a x (b x c)
#endif

//
// 4D vector specific.
//

void vec4_print(char const *name, vec4 const v);
vec4 vec4_from_vec3(vec3 const v, f32 w);

//
// 3x3 matrix math.
//

void mat3_print(char const *name, mat3 const m);
mat3 mat3_id(void);
mat3 mat3_from_cols(vec3 const c0, vec3 const c1, vec3 const c2);
mat3 mat3_from_mat4(mat4 const m);

mat3 mat3_scl(mat3 const m, f32 factor);
mat3 mat3_mul(mat3 const a, mat3 const b);

mat3 mat3_inverse(mat3 const m);
mat3 mat3_transpose(mat3 const m);
mat3 mat3_inverse_transpose(mat3 const m);

//
// 4x4 matrix math.
//

void mat4_print(char const *name, mat4 const m);
mat4 mat4_id(void);

mat4 mat4_scl(mat4 const m, f32 factor);
mat4 mat4_mul(mat4 const a, mat4 const b);

mat4 mat4_inverse(mat4 const m);
mat4 mat4_transpose(mat4 const m);
mat4 mat4_inverse_transpose(mat4 const m);

//
// Matrix-vector multiplication (considers column vectors).
//

vec3 mat3_mul_vec3(mat3 const m, vec3 const v);
vec4 mat4_mul_vec4(mat4 const m, vec4 const v);

//
// Vector-matrix multiplication (considers row vectors).
//

vec3 vec3_mul_mat3(vec3 const v, mat3 const m);
vec4 vec4_mul_mat4(vec4 const v, mat4 const m);

//
// 4x4 transformation matrices.
//

mat4 mat4_scale(vec3 const v);
mat4 mat4_translate(vec3 const v);
mat4 mat4_rotate(f32 angle_in_radians, vec3 const v);
mat4 mat4_rotate_x(f32 angle_in_radians);
mat4 mat4_rotate_y(f32 angle_in_radians);
mat4 mat4_rotate_z(f32 angle_in_radians);

mat4 mat4_lookat(vec3 const eye, vec3 const target, vec3 const up);

mat4 mat4_ortho(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far);
mat4 mat4_frustum(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far);

mat4 mat4_orthographic(f32 right, f32 top, f32 near, f32 far);
mat4 mat4_perspective(f32 fovy, f32 aspect, f32 near, f32 far);

//
// Quaternions (w, x, y, z) = w + xi + yj + zk.
//

#if 0
void quat_print(char const *name, quat const q);
quat quat_id(void);

quat quat_conjugate(quat const q);
quat quat_inverse(quat const q);
quat quat_neg(quat const q);

quat quat_add(quat const a, quat const b);
quat quat_sub(quat const a, quat const b);
quat quat_mul(quat const a, quat const b);
quat quat_scl(quat const q, f32 factor);

f32 quat_dot(quat const a, quat const b);
f32 quat_length(quat const q);
quat quat_normalize(quat const q);

quat quat_lerp(quat const a, quat const b, f32 t);
quat quat_nlerp(quat const a, quat const b, f32 t);

quat quat_from_axis_angle(vec3 const axis, f32 angle_in_radians);
#endif
