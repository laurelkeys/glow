#pragma once

//
// Common headers.
//

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

//
// Typedefs.
//

typedef int8_t i8;
typedef uint8_t u8;

typedef float f32;
typedef double f64;

typedef size_t usize;
typedef unsigned int uint;

//
// Macros.
//

#define LOOP while (true)

#define UNUSED(a) ((void) (a))

// #define IMPLIES(a, b) (!(a) || (b))

#define ARRAY_COUNT(a) (sizeof(a) / sizeof(*(a)))

#define SWAP(type, a, b) \
    do {                 \
        type tmp = (a);  \
        (a) = (b);       \
        (b) = tmp;       \
    } while (0)

//
// Miscellaneous.
//

/* Notes:
 *
 *  - OpenGL 3.3 specs:
 *      - Core Profile https://www.khronos.org/registry/OpenGL/specs/gl/glspec33.core.pdf
 *      - GLSL https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.3.30.pdf
 *
 * */
