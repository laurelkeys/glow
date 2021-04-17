#include "colorspace.h"

#define CLAMP_RGB_COMPONENTS false

f32 linear_to_srgb(f32 x) {
    return
#if CLAMP_RGB_COMPONENTS
        saturate
#endif
        (x <= 0.0031308f ? 12.92f * x : 1.055f * powf(x, (1.0f / 2.4f)) - 0.055f);
}

f32 srgb_to_linear(f32 x) {
    return
#if CLAMP_RGB_COMPONENTS
        saturate
#endif
        (x <= 0.04045f ? x / 12.92f : powf((x + 0.055f) * (1.0f / 1.055f), 2.4f));
}

#undef CLAMP_RGB_COMPONENTS

vec3 linear_rgb_to_srgb(vec3 const c) {
    return (vec3) {
        linear_to_srgb(c.x),
        linear_to_srgb(c.y),
        linear_to_srgb(c.z),
    };
}

vec3 srgb_to_linear_rgb(vec3 const c) {
    return (vec3) {
        srgb_to_linear(c.x),
        srgb_to_linear(c.y),
        srgb_to_linear(c.z),
    };
}
