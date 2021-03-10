#pragma once

#include "prelude.h"

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
Texture new_texture_from_filepath(char const *image_path, Err *err);

void bind_texture_to_unit(Texture const texture, uint const texture_unit);
