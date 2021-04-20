#include "colorspace.h"

//
// Linear RGB <-> sRGB transforms.
//

vec3 linear_rgb_to_srgb(vec3 const c) {
#define LINEAR_RGB_TO_SRGB(x) \
    ((x) <= 0.0031308f ? 12.92f * (x) : 1.055f * powf((x), (1.0f / 2.4f)) - 0.055f)

    return (vec3) {
        LINEAR_RGB_TO_SRGB(c.x),
        LINEAR_RGB_TO_SRGB(c.y),
        LINEAR_RGB_TO_SRGB(c.z),
    };

#undef LINEAR_RGB_TO_SRGB
}

vec3 srgb_to_linear_rgb(vec3 const c) {
#define SRGB_TO_LINEAR_RGB(x) \
    ((x) <= 0.04045f ? (x) / 12.92f : powf(((x) + 0.055f) * (1.0f / 1.055f), 2.4f))

    return (vec3) {
        SRGB_TO_LINEAR_RGB(c.x),
        SRGB_TO_LINEAR_RGB(c.y),
        SRGB_TO_LINEAR_RGB(c.z),
    };

#undef SRGB_TO_LINEAR_RGB
}

//
// Linear RGB <-> Oklab transforms.
//

vec3 linear_rgb_to_oklab(vec3 const c) {
    f32 const r = c.x;
    f32 const g = c.y;
    f32 const b = c.z;

    f32 const l = 0.4122214708f * r + 0.5363325363f * g + 0.0514459929f * b;
    f32 const m = 0.2119034982f * r + 0.6806995451f * g + 0.1073969566f * b;
    f32 const s = 0.0883024619f * r + 0.2817188376f * g + 0.6299787005f * b;

    f32 const l_ = cbrtf(l);
    f32 const m_ = cbrtf(m);
    f32 const s_ = cbrtf(s);

    return (vec3) {
        0.2104542553f * l_ + 0.7936177850f * m_ - 0.0040720468f * s_,
        1.9779984951f * l_ - 2.4285922050f * m_ + 0.4505937099f * s_,
        0.0259040371f * l_ + 0.7827717662f * m_ - 0.8086757660f * s_,
    };
}

vec3 oklab_to_linear_rgb(vec3 const c) {
    f32 const L = c.x; // perceived lightness
    f32 const a = c.y; // how green/red the color is
    f32 const b = c.z; // how blue/yellow the color is

    f32 const l_ = L + 0.3963377774f * a + 0.2158037573f * b;
    f32 const m_ = L - 0.1055613458f * a - 0.0638541728f * b;
    f32 const s_ = L - 0.0894841775f * a - 1.2914855480f * b;

    f32 const l = l_ * l_ * l_;
    f32 const m = m_ * m_ * m_;
    f32 const s = s_ * s_ * s_;

    return (vec3) {
        +4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s,
        -1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s,
        -0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s,
    };
}
