#pragma once

#include "prelude.h"

typedef enum TextureFilter {
    TextureFilter_Linear = 0,
    TextureFilter_Nearest,
} TextureFilter;

typedef enum TextureWrap {
    TextureWrap_Repeat = 0,
    TextureWrap_ClampToEdge,
    TextureWrap_ClampToBorder,
    TextureWrap_MirroredRepeat,
} TextureWrap;

typedef struct TextureSettings {
    bool generate_mipmap;
    TextureFilter mag_filter;
    TextureFilter min_filter;
    TextureFilter mipmap_filter;
    TextureWrap wrap_s;
    TextureWrap wrap_t;
} TextureSettings;

typedef struct TextureImage {
    u8 *data;
    int width;
    int height;
    int channels;
} TextureImage;

typedef struct Texture {
    uint id;
} Texture;

Texture new_texture_from_image(TextureImage const texture_image);
Texture new_texture_from_image_with_settings(
    TextureSettings const settings, TextureImage const texture_image);

Texture new_texture_from_filepath(char const *image_path, Err *err);
Texture new_texture_from_filepath_with_settings(
    TextureSettings const settings, char const *image_path, Err *err);

void bind_texture_to_unit(Texture const texture, uint texture_unit);
