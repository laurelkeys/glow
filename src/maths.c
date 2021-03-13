#include "maths.h"

#include <stdio.h>

// clang-format off

//
// Scalar math.
//

f32 lerp(f32 a, f32 b, f32 t) { return (1 - t) * a + t * b; }
f32 fract(f32 x) { return x - floorf(x); }
f32 clamp(f32 x, f32 x_min, f32 x_max) { return CLAMP(x, x_min, x_max); }
f32 saturate(f32 x) { return CLAMP(x, 0, 1); }

//
// 2D vector math.
//

vec2 vec2_new(f32 x, f32 y) { return (vec2) { x, y }; }
vec2 vec2_from_vec3(vec3 const v) { return vec2_new(v.x, v.y); }
void vec2_print(char const *name, vec2 const v) { printf("%s = vec2 { %3f, %3f }\n", name, v.x, v.y); }

vec2 vec2_zero(void) { return vec2_new(0, 0); }
vec2 vec2_ones(void) { return vec2_new(1, 1); }
vec2 vec2_unit_x(void) { return vec2_new(1, 0); }
vec2 vec2_unit_y(void) { return vec2_new(0, 1); }

vec2 vec2_rot_90cw(vec3 const v) { return vec2_new(v.y, -v.x); }
vec2 vec2_rot_90ccw(vec3 const v) { return vec2_new(-v.y, v.x); }

vec2 vec2_of(f32 value) { return vec2_new(value, value); }
vec2 vec2_neg(vec2 const v) { return vec2_new(-v.x, -v.y); }

vec2 vec2_min(vec2 const a, vec2 const b) { return vec2_new(MIN(a.x, b.x), MIN(a.y, b.y)); }
vec2 vec2_max(vec2 const a, vec2 const b) { return vec2_new(MAX(a.x, b.x), MAX(a.y, b.y)); }

vec2 vec2_add(vec2 const a, vec2 const b) { return vec2_new(a.x + b.x, a.y + b.y); }
vec2 vec2_sub(vec2 const a, vec2 const b) { return vec2_new(a.x - b.x, a.y - b.y); }
vec2 vec2_mul(vec2 const a, vec2 const b) { return vec2_new(a.x * b.x, a.y * b.y); }
vec2 vec2_scl(vec2 const v, f32 factor) { return vec2_new(v.x * factor, v.y * factor); }
vec2 vec2_rcp(vec2 const v) { return vec2_new(1 / v.x, 1 / v.y); }

f32 vec2_dot(vec2 const a, vec2 const b) { return a.x * b.x + a.y * b.y; }
f32 vec2_length(vec2 const v) { return sqrtf(vec2_dot(v, v)); }
vec2 vec2_normalize(vec2 const v) { return vec2_scl(v, 1 / vec2_length(v)); }

vec2 vec2_lerp(vec2 const a, vec2 const b, f32 t) { return vec2_new(lerp(a.x, b.x, t), lerp(a.y, b.y, t)); }
vec2 vec2_saturate(vec2 const v) { return vec2_new(saturate(v.x), saturate(v.y)); }

//
// 3D vector math.
//

vec3 vec3_new(f32 x, f32 y, f32 z) { return (vec3) { x, y, z }; }
vec3 vec3_from_vec2(vec2 const v, f32 z) { return vec3_new(v.x, v.y, z); }
vec3 vec3_from_vec4(vec4 const v) { return vec3_new(v.x, v.y, v.z); }
void vec3_print(char const *name, vec3 const v) { printf("%s = vec3 { %3f, %3f, %3f }\n", name, v.x, v.y, v.z); }

vec3 vec3_zero(void) { return vec3_new(0, 0, 0); }
vec3 vec3_ones(void) { return vec3_new(1, 1, 1); }
vec3 vec3_unit_x(void) { return vec3_new(1, 0, 0); }
vec3 vec3_unit_y(void) { return vec3_new(0, 1, 0); }
vec3 vec3_unit_z(void) { return vec3_new(0, 0, 1); }

vec3 vec3_cross(vec3 const a, vec3 const b) {
    return vec3_new(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x);
}

vec3 vec3_of(f32 value) { return vec3_new(value, value, value); }
vec3 vec3_neg(vec3 const v) { return vec3_new(-v.x, -v.y, -v.z); }

vec3 vec3_min(vec3 const a, vec3 const b) { return vec3_new(MIN(a.x, b.x), MIN(a.y, b.y), MIN(a.z, b.z)); }
vec3 vec3_max(vec3 const a, vec3 const b) { return vec3_new(MAX(a.x, b.x), MAX(a.y, b.y), MAX(a.z, b.z)); }

vec3 vec3_add(vec3 const a, vec3 const b) { return vec3_new(a.x + b.x, a.y + b.y, a.z + b.z); }
vec3 vec3_sub(vec3 const a, vec3 const b) { return vec3_new(a.x - b.x, a.y - b.y, a.z - b.z); }
vec3 vec3_mul(vec3 const a, vec3 const b) { return vec3_new(a.x * b.x, a.y * b.y, a.z * b.z); }
vec3 vec3_scl(vec3 const v, f32 factor) { return vec3_new(v.x * factor, v.y * factor, v.z * factor); }
vec3 vec3_rcp(vec3 const v) { return vec3_new(1 / v.x, 1 / v.y, 1 / v.z); }

f32 vec3_dot(vec3 const a, vec3 const b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
f32 vec3_length(vec3 const v) { return sqrtf(vec3_dot(v, v)); }
vec3 vec3_normalize(vec3 const v) { return vec3_scl(v, 1 / vec3_length(v)); }

vec3 vec3_lerp(vec3 const a, vec3 const b, f32 t) { return vec3_new(lerp(a.x, b.x, t), lerp(a.y, b.y, t), lerp(a.z, b.z, t)); }
vec3 vec3_saturate(vec3 const v) { return vec3_new(saturate(v.x), saturate(v.y), saturate(v.z)); }

//
// 4D vector math.
//

vec4 vec4_new(f32 x, f32 y, f32 z, f32 w) { return (vec4) { x, y, z, w }; }
vec4 vec4_from_vec3(vec3 const v, f32 w) { return vec4_new(v.x, v.y, v.z, w); }
void vec4_print(char const *name, vec4 const v) { printf("%s = vec4 { %3f, %3f, %3f, %3f }\n", name, v.x, v.y, v.z, v.w); }

vec4 vec4_zero(void) { return vec4_new(0, 0, 0, 0); }
vec4 vec4_ones(void) { return vec4_new(1, 1, 1, 1); }
vec4 vec4_unit_x(void) { return vec4_new(1, 0, 0, 0); }
vec4 vec4_unit_y(void) { return vec4_new(0, 1, 0, 0); }
vec4 vec4_unit_z(void) { return vec4_new(0, 0, 1, 0); }
vec4 vec4_unit_w(void) { return vec4_new(0, 0, 0, 1); }

vec4 vec4_of(f32 value) { return vec4_new(value, value, value, value); }
vec4 vec4_neg(vec4 const v) { return vec4_new(-v.x, -v.y, -v.z, -v.w); }

vec4 vec4_min(vec4 const a, vec4 const b) { return vec4_new(MIN(a.x, b.x), MIN(a.y, b.y), MIN(a.z, b.z), MIN(a.w, b.w)); }
vec4 vec4_max(vec4 const a, vec4 const b) { return vec4_new(MAX(a.x, b.x), MAX(a.y, b.y), MAX(a.z, b.z), MAX(a.w, b.w)); }

vec4 vec4_add(vec4 const a, vec4 const b) { return vec4_new(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }
vec4 vec4_sub(vec4 const a, vec4 const b) { return vec4_new(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }
vec4 vec4_mul(vec4 const a, vec4 const b) { return vec4_new(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w); }
vec4 vec4_scl(vec4 const v, f32 factor) { return vec4_new(v.x * factor, v.y * factor, v.z * factor, v.w * factor); }
vec4 vec4_rcp(vec4 const v) { return vec4_new(1 / v.x, 1 / v.y, 1 / v.z, 1 / v.w); }

f32 vec4_dot(vec4 const a, vec4 const b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }
f32 vec4_length(vec4 const v) { return sqrtf(vec4_dot(v, v)); }
vec4 vec4_normalize(vec4 const v) { return vec4_scl(v, 1 / vec4_length(v)); }

vec4 vec4_lerp(vec4 const a, vec4 const b, f32 t) { return vec4_new(lerp(a.x, b.x, t), lerp(a.y, b.y, t), lerp(a.z, b.z, t), lerp(a.w, b.w, t)); }
vec4 vec4_saturate(vec4 const v) { return vec4_new(saturate(v.x), saturate(v.y), saturate(v.z), saturate(v.w)); }

//
// 3x3 matrix math.
//

mat3 mat3_id(void) {
    return (mat3) { {
        { 1, 0, 0 },
        { 0, 1, 0 },
        { 0, 0, 1 },
    } };
}
mat3 mat3_from_cols(vec3 const c0, vec3 const c1, vec3 const c2) {
    return (mat3) { {
        { c0.x, c1.x, c2.x },
        { c0.y, c1.y, c2.y },
        { c0.z, c1.z, c2.z },
    } };
}
mat3 mat3_from_mat4(mat4 const m) {
    return (mat3) { {
        { m.m[0][0], m.m[0][1], m.m[0][2] },
        { m.m[1][0], m.m[1][1], m.m[1][2] },
        { m.m[2][0], m.m[2][1], m.m[2][2] },
    } };
}
void mat3_print(char const *name, mat3 const m) {
    printf("%s = mat3 { {\n", name);
    for (int i = 0; i < 3; ++i) {
        printf("{ ");
        for (int j = 0; j < 3; ++j) { printf("%3f, ", m.m[i][j]); }
        printf("},\n");
    }
    printf("} }\n");
}

mat3 mat3_scl(mat3 const m, f32 factor) {
    return (mat3) { {
        { factor * m.m[0][0], factor * m.m[0][1], factor * m.m[0][2] },
        { factor * m.m[1][0], factor * m.m[1][1], factor * m.m[1][2] },
        { factor * m.m[2][0], factor * m.m[2][1], factor * m.m[2][2] },
    } };
}
mat3 mat3_mul(mat3 const a, mat3 const b) {
    mat3 m = { 0 };
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            for (int k = 0; k < 3; ++k) { m.m[i][j] += a.m[i][k] * b.m[k][j]; }
        }
    }
    return m;
}

mat3 mat3_inverse(mat3 const m) {
    return mat3_transpose(mat3_inverse_transpose(m));
}
mat3 mat3_transpose(mat3 const m) {
    mat3 transpose;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) { transpose.m[i][j] = m.m[j][i]; }
    }
    return transpose;
}

// Reference: https://github.com/zauonlok/renderer/
static f32 mat3_determinant(mat3 const m) {
    f32 const a = +m.m[0][0] * (m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1]);
    f32 const b = -m.m[0][1] * (m.m[1][0] * m.m[2][2] - m.m[1][2] * m.m[2][0]);
    f32 const c = +m.m[0][2] * (m.m[1][0] * m.m[2][1] - m.m[1][1] * m.m[2][0]);
    return a + b + c;
}
static mat3 mat3_adjoint(mat3 const m) {
    mat3 adjoint;
    adjoint.m[0][0] = +(m.m[1][1] * m.m[2][2] - m.m[2][1] * m.m[1][2]);
    adjoint.m[0][1] = -(m.m[1][0] * m.m[2][2] - m.m[2][0] * m.m[1][2]);
    adjoint.m[0][2] = +(m.m[1][0] * m.m[2][1] - m.m[2][0] * m.m[1][1]);
    adjoint.m[1][0] = -(m.m[0][1] * m.m[2][2] - m.m[2][1] * m.m[0][2]);
    adjoint.m[1][1] = +(m.m[0][0] * m.m[2][2] - m.m[2][0] * m.m[0][2]);
    adjoint.m[1][2] = -(m.m[0][0] * m.m[2][1] - m.m[2][0] * m.m[0][1]);
    adjoint.m[2][0] = +(m.m[0][1] * m.m[1][2] - m.m[1][1] * m.m[0][2]);
    adjoint.m[2][1] = -(m.m[0][0] * m.m[1][2] - m.m[1][0] * m.m[0][2]);
    adjoint.m[2][2] = +(m.m[0][0] * m.m[1][1] - m.m[1][0] * m.m[0][1]);
    return adjoint;
}
mat3 mat3_inverse_transpose(mat3 const m) {
    mat3 const adjoint = mat3_adjoint(m);
    f32 const determinant_rcp = 1 / mat3_determinant(m);

    mat3 inverse_transpose;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            inverse_transpose.m[i][j] = adjoint.m[i][j] * determinant_rcp;
        }
    }

    return inverse_transpose;
}

//
// 4x4 matrix math.
//

mat4 mat4_id(void) {
    return (mat4) { {
        { 1, 0, 0, 0 },
        { 0, 1, 0, 0 },
        { 0, 0, 1, 0 },
        { 0, 0, 0, 1 },
    } };
}
void mat4_print(char const *name, mat4 const m) {
    printf("%s = mat4 { {\n", name);
    for (int i = 0; i < 4; ++i) {
        printf("    { ");
        for (int j = 0; j < 4; ++j) { printf("%3f, ", m.m[i][j]); }
        printf("},\n");
    }
    printf("} }\n");
}

mat4 mat4_scl(mat4 const m, f32 factor) {
    return (mat4) { {
        { factor * m.m[0][0], factor * m.m[0][1], factor * m.m[0][2], factor * m.m[0][3] },
        { factor * m.m[1][0], factor * m.m[1][1], factor * m.m[1][2], factor * m.m[1][3] },
        { factor * m.m[2][0], factor * m.m[2][1], factor * m.m[2][2], factor * m.m[2][3] },
        { factor * m.m[3][0], factor * m.m[3][1], factor * m.m[3][2], factor * m.m[3][3] },
    } };
}
mat4 mat4_mul(mat4 const a, mat4 const b) {
    mat4 m = { 0 };
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            for (int k = 0; k < 4; ++k) { m.m[i][j] += a.m[i][k] * b.m[k][j]; }
        }
    }
    return m;
}

mat4 mat4_inverse(mat4 const m) {
    return mat4_transpose(mat4_inverse_transpose(m));
}
mat4 mat4_transpose(mat4 const m) {
    mat4 transpose;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) { transpose.m[i][j] = m.m[j][i]; }
    }
    return transpose;
}

// Reference: https://github.com/zauonlok/renderer/
static f32 mat4_minor(mat4 const m, int r, int c) {
    mat3 cut_down;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            int const row = i + (int) i >= r; // i < r ? i : i + 1;
            int const col = j + (int) j >= c; // j < c ? j : j + 1;
            cut_down.m[i][j] = m.m[row][col];
        }
    }
    return mat3_determinant(cut_down);
}
static f32 mat4_cofactor(mat4 const m, int r, int c) {
    f32 const sign = (r + c) % 2 == 0 ? 1 : -1;
    f32 const minor = mat4_minor(m, r, c);
    return sign * minor;
}
static mat4 mat4_adjoint(mat4 const m) {
    mat4 adjoint;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) { adjoint.m[i][j] = mat4_cofactor(m, i, j); }
    }
    return adjoint;
}
mat4 mat4_inverse_transpose(mat4 const m) {
    mat4 const adjoint = mat4_adjoint(m);
    f32 determinant = 0;
    for (int i = 0; i < 4; ++i) { determinant += m.m[0][i] * adjoint.m[0][i]; }
    f32 const determinant_rcp = 1 / determinant;

    mat4 inverse_transpose;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            inverse_transpose.m[i][j] = adjoint.m[i][j] * determinant_rcp;
        }
    }

    return inverse_transpose;
}

//
// Matrix-vector multiplication (considers column vectors).
//

vec3 mat3_mul_vec3(mat3 const m, vec3 const v) {
    f32 product[3];
    for (int i = 0; i < 3; ++i) {
        product[i] = m.m[i][0] * v.x +
                     m.m[i][1] * v.y +
                     m.m[i][2] * v.z;
    }
    return vec3_new(product[0], product[1], product[2]);
}
vec4 mat4_mul_vec4(mat4 const m, vec4 const v) {
    f32 product[4];
    for (int i = 0; i < 4; ++i) {
        product[i] = m.m[i][0] * v.x +
                     m.m[i][1] * v.y +
                     m.m[i][2] * v.z +
                     m.m[i][3] * v.w;
    }
    return vec4_new(product[0], product[1], product[2], product[3]);
}

//
// Vector-matrix multiplication (considers row vectors).
//

vec3 vec3_mul_mat3(vec3 const v, mat3 const m) {
    f32 product[3];
    for (int j = 0; j < 3; ++j) {
        product[j] = v.x * m.m[0][j] +
                     v.y * m.m[1][j] +
                     v.z * m.m[2][j];
    }
    return vec3_new(product[0], product[1], product[2]);
}
vec4 vec4_mul_mat4(vec4 const v, mat4 const m) {
    f32 product[4];
    for (int j = 0; j < 4; ++j) {
        product[j] = v.x * m.m[0][j] +
                     v.y * m.m[1][j] +
                     v.z * m.m[2][j] +
                     v.w * m.m[3][j];
    }
    return vec4_new(product[0], product[1], product[2], product[3]);
}

//
// 4x4 transformation matrices.
//

mat4 mat4_scale(vec3 const v) {
    return (mat4) { {
        { v.x,  0,   0,  0 },
        {  0,  v.y,  0,  0 },
        {  0,   0,  v.z, 0 },
        {  0,   0,   0,  1 },
    } };
}

mat4 mat4_translate(vec3 const v) {
    return (mat4) { {
        { 1, 0, 0, v.x },
        { 0, 1, 0, v.y },
        { 0, 0, 1, v.z },
        { 0, 0, 0,  1  },
    } };
}

mat4 mat4_rotate(f32 angle_in_radians, vec3 const v) {
    f32 const c = cosf(angle_in_radians);
    f32 const s = sinf(angle_in_radians);

    vec3 const n = vec3_normalize(v);
    f32 const x2 = n.x * n.x, y2 = n.y * n.y, z2 = n.z * n.z;
    f32 const xy = n.x * n.y, yz = n.y * n.z, zx = n.z * n.x;
    f32 const xs = n.x * s,   ys = n.y * s,   zs = n.z * s;

    return (mat4) { {
        { x2 * (1-c) + c,  xy * (1-c) - zs, zx * (1-c) + ys, 0 },
        { xy * (1-c) + zs, y2 * (1-c) + c,  yz * (1-c) - xs, 0 },
        { zx * (1-c) - ys, yz * (1-c) + xs, z2 * (1-c) + c,  0 },
        {        0,               0,               0,        1 },
    } };
}

mat4 mat4_rotate_x(f32 angle_in_radians) {
    f32 const c = cosf(angle_in_radians);
    f32 const s = sinf(angle_in_radians);
    return (mat4) { {
        { 1, 0,  0, 0 },
        { 0, c, -s, 0 },
        { 0, s,  c, 0 },
        { 0, 0,  0, 1 },
    } };
}
mat4 mat4_rotate_y(f32 angle_in_radians) {
    f32 const c = cosf(angle_in_radians);
    f32 const s = sinf(angle_in_radians);
    return (mat4) { {
        {  c, 0, s, 0 },
        {  0, 1, 0, 0 },
        { -s, 0, c, 0 },
        {  0, 0, 0, 1 },
    } };
}
mat4 mat4_rotate_z(f32 angle_in_radians) {
    f32 const c = cosf(angle_in_radians);
    f32 const s = sinf(angle_in_radians);
    return (mat4) { {
        { c, -s, 0, 0 },
        { s,  c, 0, 0 },
        { 0,  0, 1, 0 },
        { 0,  0, 0, 1 },
    } };
}

// References:
//  https://github.com/zauonlok/renderer/
//  http://www.songho.ca/opengl/gl_camera.html
//  http://www.songho.ca/opengl/gl_anglestoaxes.html
//  http://www.songho.ca/opengl/gl_projectionmatrix.html

// clang-format on

mat4 mat4_lookat(vec3 const eye, vec3 const target, vec3 const up) {
    vec3 const z_axis = vec3_normalize(vec3_sub(eye, target));
    vec3 const x_axis = vec3_normalize(vec3_cross(up, z_axis));
    vec3 const y_axis = vec3_cross(z_axis, x_axis);

    return (mat4) { {
        { x_axis.x, x_axis.y, x_axis.z, -vec3_dot(x_axis, eye) },
        { y_axis.x, y_axis.y, y_axis.z, -vec3_dot(y_axis, eye) },
        { z_axis.x, z_axis.y, z_axis.z, -vec3_dot(z_axis, eye) },
        { 0, 0, 0, 1 },
    } };
}

mat4 mat4_ortho(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far) {
    f32 const x_range = right - left;
    f32 const y_range = top - bottom;
    f32 const z_range = far - near;

    assert(x_range > 0 && y_range > 0 && z_range > 0);

    return (mat4) { {
        { 2 / x_range, 0, 0, -(right + left) / x_range },
        { 0, 2 / y_range, 0, -(top + bottom) / y_range },
        { 0, 0, -2 / z_range, -(far + near) / z_range },
        { 0, 0, 0, 1 },
    } };
}
mat4 mat4_frustum(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far) {
    f32 const x_range = right - left;
    f32 const y_range = top - bottom;
    f32 const z_range = far - near;

    assert(near > 0 && far > 0);
    assert(x_range > 0 && y_range > 0 && z_range > 0);

    return (mat4) { {
        { 2 * near / x_range, 0, (left + right) / x_range, 0 },
        { 0, 2 * near / y_range, (top + bottom) / y_range, 0 },
        { 0, 0, -(far + near) / z_range, -2 * far * near / z_range },
        { 0, 0, -1, 0 },
    } };
}

mat4 mat4_orthographic(f32 right, f32 top, f32 near, f32 far) {
    f32 const z_range = far - near;

    assert(right > 0 && top > 0 && z_range > 0);

    // f32 const left = -right;
    // f32 const bottom = -top;
    // return mat4_ortho(left, right, bottom, top, near, far);

    return (mat4) { {
        { 1 / right, 0, 0, 0 },
        { 0, 1 / top, 0, 0 },
        { 0, 0, -2 / z_range, -(far + near) / z_range },
        { 0, 0, -1, 0 },
    } };
}
mat4 mat4_perspective(f32 fovy, f32 aspect, f32 near, f32 far) {
    f32 const z_range = far - near;
    f32 const tan_half_fovy_rcp = 1 / tanf(fovy / 2);

    assert(fovy > 0 && aspect > 0);
    assert(near > 0 && far > 0 && z_range > 0);

    // f32 const half_h = near * tanf(fovy / 2);
    // f32 const half_w = half_h * aspect;
    // return mat4_frustum(-half_w, half_w, -half_h, half_h, near, far);

    return (mat4) { {
        { tan_half_fovy_rcp / aspect, 0, 0, 0 },
        { 0, tan_half_fovy_rcp, 0, 0 },
        { 0, 0, -(far + near) / z_range, -2 * far * near / z_range },
        { 0, 0, -1, 0 },
    } };
}