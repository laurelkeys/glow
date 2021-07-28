#include "texture.h"

#include "console.h"

#include <stb_image.h>

#include <glad/glad.h>

// clang-format off
static int const FILTER[] = {
    [TextureFilter_Nearest] = GL_NEAREST,
    [TextureFilter_Linear ] = GL_LINEAR
};

static int const MIN_MIPMAP_FILTER[][2] = {
    [TextureFilter_Nearest][TextureFilter_Nearest] = GL_NEAREST_MIPMAP_NEAREST,
    [TextureFilter_Nearest][TextureFilter_Linear ] = GL_NEAREST_MIPMAP_LINEAR,
    [TextureFilter_Linear ][TextureFilter_Nearest] = GL_LINEAR_MIPMAP_NEAREST,
    [TextureFilter_Linear ][TextureFilter_Linear ] = GL_LINEAR_MIPMAP_LINEAR,
};

static int const WRAP[] = {
    [TextureWrap_ClampToEdge   ] = GL_CLAMP_TO_EDGE,
    [TextureWrap_ClampToBorder ] = GL_CLAMP_TO_BORDER,
    [TextureWrap_MirroredRepeat] = GL_MIRRORED_REPEAT,
    [TextureWrap_Repeat        ] = GL_REPEAT,
};

static int const FORMAT[] = {
    [TextureFormat_R   ] = GL_RED,
    [TextureFormat_Rg  ] = GL_RG,
    [TextureFormat_Rgb ] = GL_RGB,
    [TextureFormat_Rgba] = GL_RGBA,
};

STATIC_ASSERT(TextureFormat_R == 1 /* component */);
STATIC_ASSERT(TextureFormat_Rg == 2 /* components */);
STATIC_ASSERT(TextureFormat_Rgb == 3 /* components */);
STATIC_ASSERT(TextureFormat_Rgba == 4 /* components */);

static int const TARGET_TYPE[] = {
    [TextureTargetType_2D  ] = GL_TEXTURE_2D,
    [TextureTargetType_Cube] = GL_TEXTURE_CUBE_MAP,
};

static int const TARGET_TYPE_CUBE_FACE[6] = {
    [0] = GL_TEXTURE_CUBE_MAP_POSITIVE_X, [1] = GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
    [2] = GL_TEXTURE_CUBE_MAP_POSITIVE_Y, [3] = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
    [4] = GL_TEXTURE_CUBE_MAP_POSITIVE_Z, [5] = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
};
// clang-format on

static int gl_format(int channels) {
    static int const SWIZZLE_R001_TO_RRR1[] = { GL_RED, GL_RED, GL_RED, GL_ONE };
    static int const SWIZZLE_RG01_TO_RRRG[] = { GL_RED, GL_RED, GL_RED, GL_GREEN };

    // References:
    //  https://www.khronos.org/opengl/wiki/Image_Format#Legacy_Image_Formats
    //  https://www.khronos.org/opengl/wiki/Texture#Swizzle_mask

    switch (channels) {
        case 1: // replicate legacy GL_LUMINANCE
            glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, SWIZZLE_R001_TO_RRR1);
            return GL_RED;
        case 2: // replicate legacy GL_LUMINANCE_ALPHA
            glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, SWIZZLE_RG01_TO_RRRG);
            return GL_RG;
        default: // fallthrough to GL_RGB in release mode
            GLOW_WARNING("texture image with invalid number of channels: `%d`", channels);
            assert(false);
        case 3: return GL_RGB;
        case 4: return GL_RGBA;
    }
}

static int gl_internal_format(int format, bool is_srgb, bool is_highp, bool is_float) {
    assert(format == GL_RED | format == GL_RG | format == GL_RGB | format == GL_RGBA);

    // @Note: index = is_highp ? (is_float ? 3 : 2) : (is_float ? 1 : 0)
    // Reference: https://www.khronos.org/opengl/wiki/Image_Format#Required_formats
    static int const INTERNAL_FORMAT_R[4] = { GL_R8, GL_R16F, GL_R16, GL_R32F };
    static int const INTERNAL_FORMAT_RG[4] = { GL_RG8, GL_RG16F, GL_RG16, GL_RG32F };
    static int const INTERNAL_FORMAT_RGB[4] = { GL_RGB8, GL_RGB16F, GL_RGB16, GL_RGB32F };
    static int const INTERNAL_FORMAT_RGBA[4] = { GL_RGBA8, GL_RGBA16F, GL_RGBA16, GL_RGBA32F };

    if (is_srgb) {
        assert(is_highp == false);
        assert(is_float == false);
        return (format == GL_RGB) ? GL_SRGB8 : (format == GL_RGBA) ? GL_SRGB8_ALPHA8 : format;
    }

    int const index = ((!!is_highp) << 1) | (!!is_float);
    return (format == GL_RED)    ? INTERNAL_FORMAT_R[index]
           : (format == GL_RG)   ? INTERNAL_FORMAT_RG[index]
           : (format == GL_RGB)  ? INTERNAL_FORMAT_RGB[index]
           : (format == GL_RGBA) ? INTERNAL_FORMAT_RGBA[index]
                                 : format;
}

// Default value for TextureSettings.
TextureSettings const Default_TextureSettings = {
    .format = TextureFormat_Default,
    .apply_srgb_eotf = false,
    .highp_bitdepth = false,
    .floating_point = false,
    .generate_mipmap = true,
    .mag_filter = TextureFilter_Linear,
    .min_filter = TextureFilter_Nearest,
    .mipmap_filter = TextureFilter_Linear,
    .wrap_s = TextureWrap_Repeat,
    .wrap_t = TextureWrap_Repeat,
};

Texture new_texture_from_image(TextureImage const texture_image) {
    return new_texture_from_image_with_settings(Default_TextureSettings, texture_image);
}
Texture new_texture_from_image_with_settings(
    TextureSettings const settings, TextureImage const texture_image) {
    uint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    DEFER(glBindTexture(GL_TEXTURE_2D, 0)) {
        int format = gl_format(texture_image.channels);
        if (settings.format != TextureFormat_Default) {
            assert(format == FORMAT[settings.format]);
            format = FORMAT[settings.format];
        }

        int const internal_format = gl_internal_format(
            format,
            /*is_srgb*/ settings.apply_srgb_eotf,
            /*is_highp*/ settings.highp_bitdepth,
            /*is_float*/ settings.floating_point);

        assert(format != internal_format);

        glTexImage2D(
            /*target*/ GL_TEXTURE_2D,
            /*level*/ 0,
            /*internalFormat*/ internal_format,
            /*width*/ texture_image.width,
            /*height*/ texture_image.height,
            /*border*/ 0,
            /*format*/ format,
            /*type*/ settings.floating_point ? GL_FLOAT : GL_UNSIGNED_BYTE,
            /*data*/ texture_image.data);

        int min_filter;
        if (settings.generate_mipmap) {
            glGenerateMipmap(GL_TEXTURE_2D);
            min_filter = MIN_MIPMAP_FILTER[settings.min_filter][settings.mipmap_filter];
        } else {
            min_filter = FILTER[settings.min_filter];
        }

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, WRAP[settings.wrap_s]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, WRAP[settings.wrap_t]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, FILTER[settings.mag_filter]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    }

    return (Texture) { texture_id, TextureTargetType_2D, TextureMaterialType_None };
}

Texture new_texture_from_filepath(char const *image_path, Err *err) {
    return new_texture_from_filepath_with_settings(Default_TextureSettings, image_path, err);
}
Texture new_texture_from_filepath_with_settings(
    TextureSettings const settings, char const *image_path, Err *err) {
    if (*err) { return (Texture) { 0 }; }

    int width, height, channels;
    u8 *data = stbi_load(image_path, &width, &height, &channels, 0);
    if (!data) {
        GLOW_WARNING("failed to load image from path: `%s`", image_path);
        GLOW_WARNING("stbi_failure_reason() returned: `%s`", stbi_failure_reason());
        *err = Err_Stbi_Load;
        return (Texture) { 0 };
    }
    assert(1 <= channels && channels <= 4);
    assert(!stbi_is_hdr(image_path)); // @Todo: handle HDR images.

    Texture const texture = new_texture_from_image_with_settings(
        settings, (TextureImage) { data, width, height, channels });

    stbi_image_free(data);

    return texture;
}

// @Todo: allow the format, wrap mode and filter mode to be specified.
Texture new_cubemap_texture_from_images(TextureImage const texture_images[6]) {
    uint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);
    DEFER(glBindTexture(GL_TEXTURE_CUBE_MAP, 0)) {
        for (usize i = 0; i < 6; ++i) {
            int const target = TARGET_TYPE_CUBE_FACE[i];
            int const format = gl_format(texture_images[i].channels);
            // @Todo: handle HDR images (see `internalFormat` and `type`):
            glTexImage2D(
                /*target*/ target,
                /*level*/ 0,
                /*internalFormat*/ format,
                /*width*/ texture_images[i].width,
                /*height*/ texture_images[i].height,
                /*border*/ 0,
                /*format*/ format,
                /*type*/ GL_UNSIGNED_BYTE,
                /*data*/ texture_images[i].data);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    return (Texture) { texture_id, TextureTargetType_Cube, TextureMaterialType_None };
}

Texture new_cubemap_texture_from_filepaths(char const *image_paths[6], Err *err) {
    if (*err) { return (Texture) { 0 }; }

    TextureImage texture_images[6] = { 0 };
    for (usize i = 0; i < 6; ++i) {
        int width, height, channels;
        u8 *data = stbi_load(image_paths[i], &width, &height, &channels, 0);
        if (!data) {
            GLOW_WARNING("failed to load image from path: `%s`", i, image_paths[i]);
            GLOW_WARNING("stbi_failure_reason() returned: `%s`", stbi_failure_reason());
            *err = Err_Stbi_Load;
            return (Texture) { 0 };
        }
        assert(1 <= channels && channels <= 4);
        assert(!stbi_is_hdr(image_paths[i])); // @Todo: handle HDR images.
        texture_images[i] = (TextureImage) { data, width, height, channels };
    }

    Texture const texture = new_cubemap_texture_from_images(texture_images);

    for (usize i = 5; i < 6; --i) { stbi_image_free(texture_images[i].data); }

    return texture;
}

void bind_texture_to_unit(Texture const texture, uint texture_unit) {
    assert(texture_unit >= GL_TEXTURE0);
    glActiveTexture(texture_unit);
    glBindTexture(TARGET_TYPE[texture.target], texture.id);
}
