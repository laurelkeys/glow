#pragma once

#include "prelude.h"

#include "maths.h"

//
// Linear RGB <-> sRGB transforms.
//

vec3 linear_rgb_to_srgb(vec3 const c);
vec3 srgb_to_linear_rgb(vec3 const c);
