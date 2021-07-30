#include "options.h"

#include "console.h"
#include "file.h"

// Reference: https://www.duskborn.com/posts/simple-c-command-line-parser/
Options parse_args(int argc, char *argv[]) {
    // clang-format off

    #define GLOW_OPTION(s, l, n, d) assert(n <= 1);
    #include "options.inc"

    // Create variables to store each option.
    char const *const arg_is_set_flag = "set";
    #define GLOW_OPTION(s, l, n, d) char const *arg_##s = NULL;
    #include "options.inc"

    // Parse command line arguments into options.
    usize positional_arguments = 0;
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

    /*
    // Print argument values for debugging.
    usize buf_len = strlen(argv[0]) + 2; // "``"
    for (usize i = 1; i <= positional_arguments; ++i) {
        buf_len += strlen(argv[i]) + 4; // ", ``"
    }
    char *buf = calloc(buf_len + 1, sizeof(char));
    DEFER(free(buf)) {
        int offset = snprintf(buf, buf_len + 1, "`%s`", argv[0]);
        for (usize i = 1; i <= positional_arguments; ++i) {
            offset += snprintf(buf + offset, buf_len + 1 - offset, ", `%s`", argv[i]);
        }
        GLOW_DEBUG("%s (positional arguments)", buf);
    }

    #define GLOW_OPTION(s, l, n, d) \
        GLOW_DEBUG("`%s` = %s", #s, (arg_##s != 0) ? arg_##s : "not set");
    #include "options.inc"
    */

    // Exit early if we parsed the --help option.
    if (arg_h) {
        printf("\nusage: %s", point_at_last_path_component(argv[0]));
        #define GLOW_OPTION(s, l, n, d) \
            printf((!n ? " [%s]" : " [%s=<arg>]"), "--" #l);
        #include "options.inc"
        printf("\n\n"); // @Note: add any positional arguments in here, e.g.: printf(" <args>\n\n");

        int align_len = 0;
        #define GLOW_OPTION(s, l, n, d)                                                            \
        {                                                                                          \
            int len = !n ? strlen("  --" #l ", -" #s) : strlen("  --" #l "=<arg>, -" #s " <arg>"); \
            if (len > align_len) { align_len = len; }                                              \
        }
        #include "options.inc"

        #define GLOW_OPTION(s, l, n, d)                                                            \
        {                                                                                          \
            int len = !n ? strlen("  --" #l ", -" #s) : strlen("  --" #l "=<arg>, -" #s " <arg>"); \
            printf((!n ? "  %s, %s" : "  %s=<arg>, %s <arg>"), "--" #l, "-" #s);                   \
            printf(" %*s%s\n", align_len - len, "", d);                                            \
        }
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
