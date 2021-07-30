#pragma once

#include "prelude.h"

typedef struct Options {
    bool fullscreen;
    bool vsync;
    int msaa;
} Options;

Options parse_args(int argc, char *argv[]);
