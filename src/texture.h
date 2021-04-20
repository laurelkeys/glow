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

typedef enum TextureFormat {
    TextureFormat_Rgb = 0,
    TextureFormat_Rgba,
} TextureFormat;

typedef struct TextureSettings {
    TextureFormat format; // saved internally
    bool generate_mipmap;
    TextureFilter mag_filter;
    TextureFilter min_filter;
    TextureFilter mipmap_filter;
    TextureWrap wrap_s;
    TextureWrap wrap_t;
} TextureSettings;

extern TextureSettings const Default_TextureSettings;

typedef struct TextureImage {
    u8 *data;
    int width;
    int height;
    int channels;
} TextureImage;

typedef enum TextureType {
    TextureType_None = 0,
    TextureType_Diffuse,
    TextureType_Specular,
} TextureType;

typedef struct Texture {
    uint id;
    TextureType type; // defaults to TextureType_None
} Texture;

Texture new_texture_from_image(TextureImage const texture_image);
Texture new_texture_from_image_with_settings(
    TextureSettings const settings, TextureImage const texture_image);

Texture new_texture_from_filepath(char const *image_path, Err *err);
Texture new_texture_from_filepath_with_settings(
    TextureSettings const settings, char const *image_path, Err *err);

void bind_texture_to_unit(Texture const texture, uint texture_unit);
