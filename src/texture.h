#pragma once

#include "prelude.h"

// clang-format off
#define ENUM_TextureFilter(VariantX) \
    VariantX(TextureFilter_Default)  \
    VariantX(TextureFilter_Linear)   \
    VariantX(TextureFilter_Nearest)
typedef enum TextureFilter { ENUM_TextureFilter(ENUM_X_NAME) } TextureFilter;
enum { TextureFilter_Count = (0 ENUM_TextureFilter(ENUM_X_PLUS_ONE)) };
#undef ENUM_TextureFilter
// clang-format on

// clang-format off
#define ENUM_TextureWrap(VariantX)       \
    VariantX(TextureWrap_Repeat)         \
    VariantX(TextureWrap_ClampToEdge)    \
    VariantX(TextureWrap_ClampToBorder)  \
    VariantX(TextureWrap_MirroredRepeat)
typedef enum TextureWrap { ENUM_TextureWrap(ENUM_X_NAME) } TextureWrap;
enum { TextureWrap_Count = (0 ENUM_TextureWrap(ENUM_X_PLUS_ONE)) };
#undef ENUM_TextureWrap
// clang-format on

// clang-format off
#define ENUM_TextureFormat(VariantX) \
    VariantX(TextureFormat_Default)  \
    VariantX(TextureFormat_R)        \
    VariantX(TextureFormat_Rg)       \
    VariantX(TextureFormat_Rgb)      \
    VariantX(TextureFormat_Rgba)
typedef enum TextureFormat { ENUM_TextureFormat(ENUM_X_NAME) } TextureFormat;
enum { TextureFormat_Count = (0 ENUM_TextureFormat(ENUM_X_PLUS_ONE)) };
#undef ENUM_TextureFormat
// clang-format on

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

// clang-format off
#define ENUM_TextureMaterialType(VariantX) \
    VariantX(TextureMaterialType_None)     \
    VariantX(TextureMaterialType_Diffuse)  \
    VariantX(TextureMaterialType_Specular) \
    VariantX(TextureMaterialType_Ambient)  \
    VariantX(TextureMaterialType_Normal)   \
    VariantX(TextureMaterialType_Height)
typedef enum TextureMaterialType { ENUM_TextureMaterialType(ENUM_X_NAME) } TextureMaterialType;
enum { TextureMaterialType_Count = (0 ENUM_TextureMaterialType(ENUM_X_PLUS_ONE)) };
#undef ENUM_TextureMaterialType
// clang-format on

// clang-format off
#define ENUM_TextureTargetType(VariantX) \
    VariantX(TextureTargetType_2D)       \
    VariantX(TextureTargetType_Cube)
typedef enum TextureTargetType { ENUM_TextureTargetType(ENUM_X_NAME) } TextureTargetType;
enum { TextureTargetType_Count = (0 ENUM_TextureTargetType(ENUM_X_PLUS_ONE)) };
#undef ENUM_TextureTargetType
// clang-format on

typedef struct Texture {
    uint id;
    TextureTargetType target;
    TextureMaterialType material;
} Texture;

Texture new_texture_from_image(TextureImage const texture_image, TextureSettings const settings);
Texture new_texture_from_filepath(char const *path, TextureSettings const settings, Err *err);

// @Note: the expected order for the 6 faces is: Right, Left, Top, Bottom, Front, Back.
// Which follows the GL_TEXTURE_CUBE_MAP_*_* constants for: +X, -X, +Y, -Y, +Z, and -Z.
Texture new_cubemap_texture_from_images(
    TextureImage const texture_images[6], TextureSettings const settings);
Texture new_cubemap_texture_from_filepaths(
    char const *paths[6], TextureSettings const settings, Err *err);

void bind_texture_to_unit(Texture const texture, uint texture_unit);
