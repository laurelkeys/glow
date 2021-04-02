#include "file.h"

#include <stdio.h>
#include <string.h>

usize file_size_in_bytes(FILE *fp) {
    fpos_t fpos;
    fgetpos(fp, &fpos);
    fseek(fp, 0, SEEK_END);
    long const fsize = ftell(fp);
    fsetpos(fp, &fpos); // restore the initial file position
    return (usize) fsize;
}

char *alloc_data_from_filepath(char const *path, Err *err) {
    FILE *fp = fopen(path, "rb");
    if (!fp) { return (*err = Err_Fopen, NULL); }

    usize const fsize = file_size_in_bytes(fp);
    char *data = calloc(fsize + 1, sizeof(char));

    if (data) {
        fread(data, 1, fsize, fp);
    } else {
        *err = Err_Calloc;
    }

    fclose(fp);
    return data;
}

char *alloc_str_copy(char const *str) {
    usize const len = strlen(str);
    char *str_copy = malloc(len + 1);
    if (!str_copy) { return NULL; }
    return memcpy(str_copy, str, len + 1);
}

void replace_back_with_forward_slashes(char *path) {
    // Replace all occurences of '\\' with '/'.
    char *backslash = strchr(path, '\\');
    while (backslash) {
        *backslash = '/';
        backslash = strchr(backslash + 1, '\\');
    }
}

#ifdef _WIN32
#define SLASH_NEQ(chr) (('\\' != chr) && ('/' != chr))
#define SLASH_EQ(chr) (('\\' == chr) || ('/' == chr))
#else
#define SLASH_NEQ(chr) ('/' != chr)
#define SLASH_EQ(chr) ('/' == chr)
#endif

void terminate_at_last_path_component(char *path) {
    usize const len = strlen(path);
    if (len == 0) { return; }
    char *last = &path[len - 1]; // @Robustness: handle drive letter

    // Skip trailing slashes.
    while (last != path && SLASH_EQ(*last)) { last -= 1; }

    // Find the first non-trailing slash.
    while (last != path && SLASH_NEQ(*last)) { last -= 1; }

    if (last == path && SLASH_EQ(*path)) {
        *(last + 1) = '\0';
    } else {
        *last = '\0';
    }
}

char *point_at_last_path_component(char *path) {
    usize const len = strlen(path);
    if (len == 0) { return path; }
    char *last = &path[len - 1]; // @Robustness: handle drive letter

    // Skip trailing slashes.
    while (last != path && SLASH_EQ(*last)) { last -= 1; }

    char *first = last;
    *(last + 1) = '\0';

    // Find the first non-trailing slash.
    while (first != path && SLASH_NEQ(*first)) { first -= 1; }

    return (first == path) ? path : (first + 1);
}

#undef SLASH_EQ
#undef SLASH_NEQ
