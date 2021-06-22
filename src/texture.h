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
    TextureFormat format;
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

typedef enum TextureMaterialType {
    TextureMaterialType_None = 0,
    TextureMaterialType_Ambient,
    TextureMaterialType_Diffuse,
    TextureMaterialType_Specular,
    // @Fixme: add new values where needed (and tag modified places with @Volatile!)
    TextureMaterialType_Normal,
    TextureMaterialType_Height,
} TextureMaterialType;

typedef enum TextureTargetType {
    TextureTargetType_2D = 0, // GL_TEXTURE_2D
    TextureTargetType_Cube, // GL_TEXTURE_CUBE_MAP
} TextureTargetType;

typedef struct Texture {
    uint id;
    TextureTargetType target; // defaults to TextureTargetType_2D
    TextureMaterialType material; // defaults to TextureMaterialType_None
} Texture;

Texture new_texture_from_image(TextureImage const texture_image);
Texture new_texture_from_image_with_settings(
    TextureSettings const settings, TextureImage const texture_image);

Texture new_texture_from_filepath(char const *image_path, Err *err);
Texture new_texture_from_filepath_with_settings(
    TextureSettings const settings, char const *image_path, Err *err);

void bind_texture_to_unit(Texture const texture, uint texture_unit);

char const *sampler_name_from_texture_material(TextureMaterialType const material);

#if 1
// @Note: the expected order for the 6 faces is: Right, Left, Top, Bottom, Front, Back.
// Which follows the GL_TEXTURE_CUBE_MAP_*_* constants for: +X, -X, +Y, -Y, +Z, and -Z.
Texture new_cubemap_texture_from_images(TextureImage const texture_images[6]);
Texture new_cubemap_texture_from_filepaths(char const *image_paths[6], Err *err);
#endif
