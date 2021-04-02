#pragma once

#include "prelude.h"

#include <stdio.h>

usize file_size_in_bytes(FILE *fp);

char *alloc_data_from_filepath(char const *path, Err *err);

char *alloc_str_copy(char const *str);

void replace_back_with_forward_slashes(char *path);
void terminate_at_last_path_component(char *path);
char *point_at_last_path_component(char *path);
