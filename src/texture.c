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
    switch (channels) {
        case 1: return GL_RED;
        case 2: return GL_RG;
        default: // fallthrough to GL_RGB in release mode
            GLOW_WARNING("texture image with invalid number of channels: `%d`", channels);
            assert(false);
        case 3: return GL_RGB;
        case 4: return GL_RGBA;
    }
}

static int gl_internal_format(int format, bool is_srgb, bool is_highp, bool is_float) {
    assert((format == GL_RED) | (format == GL_RG) | (format == GL_RGB) | (format == GL_RGBA));

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

Texture new_texture_from_image(TextureImage const texture_image, TextureSettings const settings) {
    // Default initialize zeroed values.
#define DEFAULT(value) ((value) == 0)
#define VALUE_OR(value, default) (DEFAULT(value) ? (default) : (value))

    int const expected_format = gl_format(texture_image.channels);
    int const format = DEFAULT(settings.format) ? expected_format : FORMAT[settings.format];
    int const internal_format = gl_internal_format(
        format, settings.apply_srgb_eotf, settings.highp_bitdepth, settings.floating_point);

    assert(DEFAULT(settings.format) || FORMAT[settings.format] == expected_format);
    assert(internal_format != format); // we want the internal format to be sized

    int const mag_filter = FILTER[VALUE_OR(settings.mag_filter, TextureFilter_Linear)];
    int const min_filter =
        settings.generate_mipmap
            ? MIN_MIPMAP_FILTER[VALUE_OR(settings.min_filter, TextureFilter_Nearest)]
                               [VALUE_OR(settings.mipmap_filter, TextureFilter_Linear)]
            : FILTER[VALUE_OR(settings.min_filter, TextureFilter_Linear)];

    int const type = GL_UNSIGNED_BYTE;
    //  settings.floating_point
    //      ? (settings.highp_bitdepth ? GL_FLOAT : GL_HALF_FLOAT)
    //      : (settings.highp_bitdepth ? GL_UNSIGNED_SHORT : GL_UNSIGNED_BYTE);
    assert(!settings.highp_bitdepth && !settings.floating_point); // @Temporary

#undef VALUE_OR
#undef DEFAULT

    // Create the 2D texture target handle.
    uint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    DEFER(glBindTexture(GL_TEXTURE_2D, 0)) {
        glTexImage2D(
            /*target*/ GL_TEXTURE_2D,
            /*level*/ 0,
            /*internalFormat*/ internal_format,
            /*width*/ texture_image.width,
            /*height*/ texture_image.height,
            /*border*/ 0,
            /*format*/ format,
            /*type*/ type,
            /*data*/ texture_image.data);

        if (settings.generate_mipmap) { glGenerateMipmap(GL_TEXTURE_2D); }

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, WRAP[settings.wrap]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, WRAP[settings.wrap]);

        // Reference: https://www.khronos.org/opengl/wiki/Image_Format#Legacy_Image_Formats
        if (format == GL_RED) { // replicate legacy GL_LUMINANCE
            static int const SWIZZLE_R001_TO_RRR1[] = { GL_RED, GL_RED, GL_RED, GL_ONE };
            glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, SWIZZLE_R001_TO_RRR1);
        } else if (format == GL_RG) { // replicate legacy GL_LUMINANCE_ALPHA
            static int const SWIZZLE_RG01_TO_RRRG[] = { GL_RED, GL_RED, GL_RED, GL_GREEN };
            glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, SWIZZLE_RG01_TO_RRRG);
        }
    }

    return (Texture) { texture_id, TextureTargetType_2D, TextureMaterialType_None };
}

Texture new_texture_from_filepath(char const *path, TextureSettings const settings, Err *err) {
    if (*err) { return (Texture) { 0 }; }

    int width, height, channels;
    u8 *data = stbi_load(path, &width, &height, &channels, 0);
    if (!data) {
        GLOW_WARNING("failed to load image from path: `%s`", path);
        GLOW_WARNING("stbi_failure_reason() returned: `%s`", stbi_failure_reason());
        *err = Err_Stbi_Load;
        return (Texture) { 0 };
    }
    assert(1 <= channels && channels <= 4);
    assert(!stbi_is_hdr(path)); // @Todo: handle HDR images.

    Texture const texture =
        new_texture_from_image((TextureImage) { data, width, height, channels }, settings);

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

Texture new_cubemap_texture_from_filepaths(char const *paths[6], Err *err) {
    if (*err) { return (Texture) { 0 }; }

    TextureImage texture_images[6] = { 0 };
    for (usize i = 0; i < 6; ++i) {
        int width, height, channels;
        u8 *data = stbi_load(paths[i], &width, &height, &channels, 0);
        if (!data) {
            GLOW_WARNING("failed to load image from path: `%s`", i, paths[i]);
            GLOW_WARNING("stbi_failure_reason() returned: `%s`", stbi_failure_reason());
            *err = Err_Stbi_Load;
            return (Texture) { 0 };
        }
        assert(1 <= channels && channels <= 4);
        assert(!stbi_is_hdr(paths[i])); // @Todo: handle HDR images.
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
