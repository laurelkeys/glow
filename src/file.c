#include "file.h"

#include <string.h>

usize file_size_in_bytes(FILE *fp) {
    fpos_t fpos;
    fgetpos(fp, &fpos);
    fseek(fp, 0, SEEK_END);
    long const fsize = ftell(fp);
    fsetpos(fp, &fpos); // restore the initial file position
    return (usize) fsize;
}

char *alloc_human_readable_size_str(usize size_in_bytes, Err *err) {
    if (*err) { return NULL; }

    // Reference: https://github.com/vkoskiv/c-ray/blob/master/src/utils/fileio.c

    f64 const size_in_kilobytes = size_in_bytes / 1000.0;
    f64 const size_in_megabytes = size_in_kilobytes / 1000.0;
    f64 const size_in_gigabytes = size_in_megabytes / 1000.0;
    f64 const size_in_terabytes = size_in_gigabytes / 1000.0;
    f64 const size_in_petabytes = size_in_terabytes / 1000.0;

    usize const buf_size = 64; // 64 seems large enough..
    char *buf = calloc(buf_size, sizeof(char));
    if (!buf) {
        *err = Err_Calloc;
        return NULL;
    }

    if (size_in_terabytes >= 1000) {
        snprintf(buf, buf_size, "%.02fPB", size_in_petabytes); // PB
    } else if (size_in_gigabytes >= 1000) {
        snprintf(buf, buf_size, "%.02fTB", size_in_terabytes); // TB
    } else if (size_in_megabytes >= 1000) {
        snprintf(buf, buf_size, "%.02fGB", size_in_gigabytes); // GB
    } else if (size_in_kilobytes >= 1000) {
        snprintf(buf, buf_size, "%.02fMB", size_in_megabytes); // MB
    } else if (size_in_bytes >= 1000) {
        snprintf(buf, buf_size, "%.02fkB", size_in_kilobytes); // kB
    } else {
        snprintf(buf, buf_size, "%zudB", size_in_bytes); // B
    }

    return buf;
}

char *alloc_data_from_filepath(char const *path, Err *err) {
    if (*err) { return NULL; }

    FILE *fp = fopen(path, "rb");
    if (!fp) {
        *err = Err_Fopen;
        return NULL;
    }

    usize const fsize = file_size_in_bytes(fp);
    char *data = calloc(fsize + 1, sizeof(char));
    if (!data) {
        fclose(fp);
        *err = Err_Calloc;
        return NULL;
    }

    fread(data, 1, fsize, fp);
    fclose(fp);
    return data;
}

char *alloc_str_copy(char const *str, Err *err) {
    if (*err || !str) { return NULL; }
    usize const len = strlen(str);
    char *str_copy = calloc(len + 1, sizeof(char));
    if (!str_copy) {
        *err = Err_Calloc;
        return NULL;
    }
    return memcpy(str_copy, str, len + 1);
}

void replace_back_with_forward_slashes_inplace(char *path) {
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

char const *point_at_last_path_component(char const *path) {
    usize const len = !path ? 0 : strlen(path);
    if (len == 0) { return path; }

    char const *last = &path[len - 1]; // @Robustness: handle drive letter

    // Skip trailing slashes.
    while (last != path && SLASH_EQ(*last)) { last -= 1; }

    char const *first = last;
    // *(last + 1) = '\0'; // we should do this if path wasn't a const pointer...

    // Find the first non-trailing slash.
    while (first != path && SLASH_NEQ(*first)) { first -= 1; }

    return first + (path == first ? 0 : 1);
}

void terminate_at_last_path_component_inplace(char *path) {
    char *p = (char *) point_at_last_path_component(path);
    if (p == path && SLASH_EQ(*path)) { ++p; }
    *p = '\0';
}

#undef SLASH_EQ
#undef SLASH_NEQ
