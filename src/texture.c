#include "texture.h"

#include "console.h"
#include "opengl.h"

#include <stb_image.h>

// clang-format off
static int const FILTER[] = { [TextureFilter_Nearest] = GL_NEAREST, [TextureFilter_Linear] = GL_LINEAR };
static int const MIN_MIPMAP_FILTER[][2] = {
    [TextureFilter_Nearest][TextureFilter_Nearest] = GL_NEAREST_MIPMAP_NEAREST,
    [TextureFilter_Nearest][TextureFilter_Linear ] = GL_NEAREST_MIPMAP_LINEAR,
    [TextureFilter_Linear ][TextureFilter_Nearest] = GL_LINEAR_MIPMAP_NEAREST,
    [TextureFilter_Linear ][TextureFilter_Linear ] = GL_LINEAR_MIPMAP_LINEAR,
};
// clang-format on

static int const WRAP[] = {
    [TextureWrap_ClampToEdge] = GL_CLAMP_TO_EDGE,
    [TextureWrap_ClampToBorder] = GL_CLAMP_TO_BORDER,
    [TextureWrap_MirroredRepeat] = GL_MIRRORED_REPEAT,
    [TextureWrap_Repeat] = GL_REPEAT,
};

static int const INTERNAL_FORMAT[] = {
    [TextureFormat_Rgb] = GL_RGB,
    [TextureFormat_Rgba] = GL_RGBA,
};

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

// Default value for TextureSettings.
TextureSettings const Default_TextureSettings = {
    .format = TextureFormat_Rgb,
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
    {
        int const internal_format = INTERNAL_FORMAT[settings.format];
        int const format = gl_format(texture_image.channels);
        if (internal_format != format) {
            GLOW_WARNING(
                "%dx%dx%d texture image has OpenGL format=`0x%x` but internalFormat=`0x%x`",
                texture_image.width,
                texture_image.height,
                texture_image.channels,
                format,
                internal_format);
        }

        glTexImage2D(
            /*target*/ GL_TEXTURE_2D,
            /*level*/ 0,
            /*internalFormat*/ internal_format,
            /*width*/ texture_image.width,
            /*height*/ texture_image.height,
            /*border*/ 0,
            /*format*/ format,
            /*type*/ GL_UNSIGNED_BYTE,
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
    glBindTexture(GL_TEXTURE_2D, 0);

    return (Texture) { texture_id, TextureType_None };
}

Texture new_texture_from_filepath(char const *image_path, Err *err) {
    return new_texture_from_filepath_with_settings(Default_TextureSettings, image_path, err);
}
Texture new_texture_from_filepath_with_settings(
    TextureSettings const settings, char const *image_path, Err *err) {
    int width, height, channels;
    u8 *data = stbi_load(image_path, &width, &height, &channels, 0);
    if (!data) {
        GLOW_WARNING("failed to load image from path: `%s`", image_path);
        GLOW_WARNING("stbi_failure_reason() returned: `%s`", stbi_failure_reason());
        return (*err = Err_Stbi_Load, (Texture) { 0 });
    }
    assert(1 <= channels && channels <= 4);
    Texture texture = new_texture_from_image_with_settings(
        settings, (TextureImage) { data, width, height, channels });
    stbi_image_free(data);
    return texture;
}

void bind_texture_to_unit(Texture const texture, uint texture_unit) {
    glActiveTexture(texture_unit);
    glBindTexture(GL_TEXTURE_2D, texture.id);
}
