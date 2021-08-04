// clang-format off

#ifndef GLOW_OPTION
#error GLOW_OPTION(short-name, long-name, number-of-trailing-args, description) was not defined!
#endif

GLOW_OPTION(f, fullscreen, 0, "Fullscreen mode   (default: false)")
GLOW_OPTION(v, vsync,      0, "Enable V-Sync     (default: false)")
GLOW_OPTION(u, no_ui,      0, "Disable the GUI   (default: false)")
GLOW_OPTION(m, msaa,       1, "Set MSAA samples  (default: 0)")
GLOW_OPTION(h, help,       0, "Print all the options and exit")

#undef GLOW_OPTION
