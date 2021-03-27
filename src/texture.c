#include "texture.h"

#include "console.h"
#include "opengl.h"

#include <stb_image.h>

Texture new_texture_from_image(TextureImage const texture_image) {
    uint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(
        /*target*/ GL_TEXTURE_2D,
        /*level*/ 0,
        /*internalformat*/ GL_RGB,
        /*width*/ texture_image.width,
        /*height*/ texture_image.height,
        /*border*/ 0,
        /*format*/ texture_image.channels == 3 ? GL_RGB : GL_RGBA,
        /*type*/ GL_UNSIGNED_BYTE,
        /*data*/ texture_image.data);
    glGenerateMipmap(GL_TEXTURE_2D);
    return (Texture) { texture_id };
}

Texture new_texture_from_filepath(char const *image_path, Err *err) {
    int width, height, channels;
    u8 *data = stbi_load(image_path, &width, &height, &channels, 0);
    if (!data) {
        GLOW_WARNING("failed to load image from path: '%s'", image_path);
        return (*err = Err_Stbi_Load, (Texture) { 0 });
    }
    if (channels != 3 && channels != 4) {
        GLOW_WARNING("expected an RGB or RGBA image, but got %d channels instead", channels);
        assert(false);
    }
    Texture texture = new_texture_from_image((TextureImage) { data, width, height, channels });
    stbi_image_free(data);
    return texture;
}

void bind_texture_to_unit(Texture const texture, uint texture_unit) {
    glActiveTexture(texture_unit);
    glBindTexture(GL_TEXTURE_2D, texture.id);
}
