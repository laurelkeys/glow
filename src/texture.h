#pragma once

#include "prelude.h"

typedef enum TextureFilter {
    TextureFilter_Nearest,
    TextureFilter_Linear,
} TextureFilter;

typedef enum TextureWrap {
    TextureWrap_ClampToEdge,
    TextureWrap_ClampToBorder,
    TextureWrap_MirroredRepeat,
    TextureWrap_Repeat,
} TextureWrap;

typedef struct TextureSettings {
    bool generate_mipmap;
    TextureFilter mipmap_filter;
    TextureFilter min_filter;
    TextureFilter mag_filter;
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
