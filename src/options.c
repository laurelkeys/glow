#include "options.h"

#include "console.h"
#include "file.h"

Options parse_args(int argc, char *argv[]) {
    // Reference: https://www.duskborn.com/posts/simple-c-command-line-parser/

    // clang-format off
    // GLOW_OPTION(short-name, long-name, number-of-trailing-args, description)

    #define GLOW_OPTION(s, l, n, d) assert(n <= 1);
    #include "options.inc"

    // Create variables to store each option.
    char const *const arg_is_set_flag = "set";
    #define GLOW_OPTION(s, l, n, d) char const *arg_##s = NULL;
    #include "options.inc"

    // Parse command line arguments into options.
    int positional_arguments = 0;
    for (int i = 1; i < argc; ++i) {
        #define GLOW_OPTION(s, l, n, d)                                                      \
        {                                                                                    \
            if (!strncmp("-" #s, argv[i], strlen("-" #s))) {                                 \
                arg_##s = !n ? arg_is_set_flag : argv[++i];                                  \
                continue;                                                                    \
            } else if (!strncmp(!n ? "--" #l : "--" #l "=", argv[i], strlen("--" #l "="))) { \
                arg_##s = !n ? arg_is_set_flag : (argv[i] + strlen("--" #l "="));            \
                continue;                                                                    \
            }                                                                                \
        }
        #include "options.inc"

        // Save positional arguments (starting at 1 to preserve argv[0] for --help).
        argv[++positional_arguments] = argv[i];
    }

#ifndef NDEBUG
    // @Cleanup: why can't we just have a simple way to join strings in C?
    usize buf_count = 0;
    for (int i = 1; i <= positional_arguments; ++i) {
        buf_count += strlen(argv[i]) + sizeof(", ``");
    }
    char *buf = calloc(buf_count + strlen(argv[0]) + 2, sizeof(char));
    int offset = sprintf(buf, "`%s`", argv[0]);
    DEFER(free(buf)) {
        for (int i = 1; i <= positional_arguments; ++i) {
            offset += snprintf(buf + offset, buf_count - offset, ", `%s`", argv[i]);
        }
        GLOW_DEBUG("%s (positional arguments)", buf);
    }

    #define GLOW_OPTION(s, l, n, d) \
        GLOW_DEBUG("`%s` = %s", #s, (arg_##s != 0) ? arg_##s : "not set");
    #include "options.inc"
#endif

    // Exit early if the --help option was parsed.
    if (arg_h) {
        printf("\nusage: %s", point_at_last_path_component(argv[0]));

        #define GLOW_OPTION(s, l, n, d) \
            printf((!n ? " [%s]" : " [%s=<arg>]"), "--" #l);
        #include "options.inc"

        printf(" <args>\n\n"); // positional arguments

        #define GLOW_OPTION(s, l, n, d) \
            printf((!n ? "  %s, %s %s\n" : "  %s=<arg>, %s <arg> %s\n"), "--" #l, "-" #s, d);
        #include "options.inc"

        exit(EXIT_SUCCESS);
    }
    // clang-format on

    Options options = { 0 };

    if (arg_f == arg_is_set_flag) { options.fullscreen = true; }
    if (arg_v == arg_is_set_flag) { options.vsync = true; }
    if (arg_m) {
        assert(strlen(arg_m) <= 2);
        options.msaa = atoi(arg_m);
    }

    return options;
}
