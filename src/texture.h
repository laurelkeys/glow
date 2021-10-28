#pragma once

#include "prelude.h"

typedef enum TextureFilter {
    TextureFilter_Default = 0,
    TextureFilter_Linear,
    TextureFilter_Nearest,
} TextureFilter;

typedef enum TextureWrap {
    TextureWrap_Repeat = 0,
    TextureWrap_ClampToEdge,
    TextureWrap_ClampToBorder,
    TextureWrap_MirroredRepeat,
} TextureWrap;

typedef enum TextureFormat {
    TextureFormat_Default = 0,
    TextureFormat_R,
    TextureFormat_Rg,
    TextureFormat_Rgb,
    TextureFormat_Rgba,
} TextureFormat;

typedef struct TextureSettings {
    TextureFormat format;
    bool flip_vertically; // @Note: only applied into new_texture_from_filepath()
    bool apply_srgb_eotf; // @Note: assumes 8-bit-per-channel sRGB or sRGBA types
    bool highp_bitdepth; // GL_UNSIGNED_BYTE 8 -> 16 bits, GL_FLOAT 16 -> 32 bits
    bool floating_point; // GL_UNSIGNED_BYTE if false else GL_FLOAT
    bool generate_mipmap;
    TextureFilter mag_filter;
    TextureFilter min_filter;
    TextureFilter mipmap_filter;
    TextureWrap wrap;
} TextureSettings;

typedef struct TextureImage {
    u8 *data;
    int width;
    int height;
    int channels;
} TextureImage;

// @Volatile: sync with mesh.c and models.c.
// @Refactor: move this to mesh.h instead, simply as MaterialType.
typedef enum TextureMaterialType {
    TextureMaterialType_None = 0,
    TextureMaterialType_Diffuse,
    TextureMaterialType_Specular,
    TextureMaterialType_Ambient,
    TextureMaterialType_Normal,
    TextureMaterialType_Height,
    /* TextureMaterialType_Emissive,
    TextureMaterialType_Shininess,
    TextureMaterialType_Light,
    TextureMaterialType_Displacement, */
} TextureMaterialType;

typedef enum TextureTarget {
    TextureTarget_2D = 0,
    TextureTarget_Cube,
} TextureTarget;

typedef struct Texture {
    uint id;
    TextureTarget target;
    TextureMaterialType material_type;
} Texture;

Texture new_texture_from_image(TextureImage const image, TextureSettings const settings);
Texture new_texture_from_filepath(char const *path, TextureSettings const settings, Err *err);

// @Note: the expected order for the 6 faces is: Right, Left, Top, Bottom, Front, Back.
// Which follows the GL_TEXTURE_CUBE_MAP_*_* constants for: +X, -X, +Y, -Y, +Z, and -Z.
Texture
new_cubemap_texture_from_images(TextureImage const images[6], TextureSettings const settings);
Texture new_cubemap_texture_from_filepaths(
    char const *paths[6], TextureSettings const settings, Err *err);

void bind_texture_to_unit(Texture const texture, uint texture_unit);
