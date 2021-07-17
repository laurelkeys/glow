#pragma once

#include "prelude.h"

#include <stdio.h>

#ifdef _WIN32
#define SLASH "\\"
#define SLASH_CHAR '\\'
#else
#define SLASH "/"
#define SLASH_CHAR '/'
#endif

usize file_size_in_bytes(FILE *fp);

char *alloc_human_readable_size_str(usize size_in_bytes);

char *alloc_data_from_filepath(char const *path, Err *err);

char *alloc_str_copy(char const *str);

void replace_back_with_forward_slashes_inplace(char *path);

void terminate_at_last_path_component_inplace(char *path);

char const *point_at_last_path_component(char const *path);
