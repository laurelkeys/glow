#pragma once

#include "prelude.h"

#include <float.h>
#include <math.h>

//
// Constants.
//

#define M_PI 3.14159265358979323846
#define M_TAU 6.28318530717958647692

//
// Macro functions.
//

#define COMPARE(a, b) (((b) < (a)) - ((a) < (b)))
#define SIGN_OF(a) COMPARE((a), 0)

#define RADIANS(degrees) ((degrees) * (M_TAU / 360.0))
#define DEGREES(radians) ((radians) * (360.0 / M_TAU))

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define MIN3(a, b, c) (MIN(MIN((a), (b)), (c)))
#define MAX3(a, b, c) (MAX(MAX((a), (b)), (c)))
#define MIN4(a, b, c, d) (MIN(MIN((a), (b)), MIN((c), (d))))
#define MAX4(a, b, c, d) (MAX(MAX((a), (b)), MAX((c), (d))))

#define IS_ZERO(a) (((double) (a) == 0.0) || (fabs((double) (a)) < (double) FLT_EPSILON))
#define IS_EQ(a, b) \
    (((double) (a) == (double) (b)) || (fabs((double) ((a) - (b))) < (double) FLT_EPSILON))

#define CLAMP(x, a, b) (((x) < (a)) ? (a) : ((x) > (b)) ? (b) : (x))
#define CLAMP_MAX(x, b) (((x) > (b)) ? (b) : (x))
#define CLAMP_MIN(x, a) (((x) < (a)) ? (a) : (x))
