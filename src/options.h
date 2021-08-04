#pragma once

#include "prelude.h"

typedef struct Options {
    bool fullscreen;
    bool vsync;
    bool no_ui;
    int msaa;
} Options;

Options parse_args(int argc, char *argv[]);
