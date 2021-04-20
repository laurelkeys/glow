#pragma once

#include "prelude.h"

#include "maths.h"

// @Note: we assume that everything (input or output) color-related is in the sRGB color space.
// Hence, if we want to match the behavior of real light and materials, linear RGB should be used.
// While, if we want to match the human perception of color, Oklab should be used
// (which is a perceptual color space that assumes D65 whitepoint, same as sRGB).
// Reference: https://bottosson.github.io/posts/oklab/

// @Todo: represent linear RGB and sRGB as HSV/HSL, and Oklab as LCh.

//
// Linear RGB <-> sRGB transforms.
//

vec3 linear_rgb_to_srgb(vec3 const c); // device color space output
vec3 srgb_to_linear_rgb(vec3 const c); // linear color space output

//
// Linear RGB <-> Oklab transforms.
//

vec3 linear_rgb_to_oklab(vec3 const c); // perceptual color space output
vec3 oklab_to_linear_rgb(vec3 const c); // linear color space output
