#pragma once

#include "prelude.h"

#include <stdarg.h>
#include <stdio.h>

inline void glow_log(char const* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fflush(stderr);
}

#define GLOW_LOG(fmt, ...) (void) (glow_log("[glow] " fmt ".\n", ##__VA_ARGS__))
#define GLOW_DEBUG(fmt, ...) \
    (void) (glow_log("\033[0;90m[glow] Debug: " fmt ".\033[0m\n", ##__VA_ARGS__))
#define GLOW_ERROR(fmt, ...) \
    (void) (glow_log("\033[0;31m[glow] Error: " fmt ".\033[0m\n", ##__VA_ARGS__))
#define GLOW_WARNING(fmt, ...) \
    (void) (glow_log("\033[0;33m[glow] Warning: " fmt ".\033[0m\n", ##__VA_ARGS__))
