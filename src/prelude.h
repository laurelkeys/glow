#pragma once

//
// Common headers.
//

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

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

#define ARRAY_LEN(a) (sizeof(a) / sizeof(*(a)))
#define ARRAY_END(a) ((a) + ARRAY_LEN(a))

#define CONCATIFY_(a, b) a##b
#define CONCATIFY(a, b) CONCATIFY_(a, b)

#define STRINGIFY_(a) #a
#define STRINGIFY(a) STRINGIFY_(a)

#define SWAP(type, a, b) \
    do {                 \
        type tmp = (a);  \
        (a) = (b);       \
        (b) = tmp;       \
    } while (0)

/*
#define FOR(type, begin, end)  \
    for (                      \
        struct {               \
            usize index;       \
            type *value;       \
        } it = { 0, (begin) }; \
        it.value != (end);     \
        ++it.index, ++it.value)
*/

//
// Miscellaneous.
//

typedef enum Err {
    Err_None = 0, // EXIT_SUCCESS
    Err_Unkown = 1, // EXIT_FAILURE

    Err_Glfw_Init,
    Err_Glfw_Window,

    Err_Glad_Init,

    Err_Shader_Compile,
    Err_Shader_Link,

    Err_Stbi_Load,

    Err_Assimp_Import,
    Err_Assimp_Get_Texture,

    Err_Fopen,
    Err_Malloc,
    Err_Calloc,
    Err_Realloc,
} Err;
