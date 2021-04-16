#version 330 core

out vec4 fragColor;

// @Volatile: use the same values as defined in camera.h.
const float Z_FAR = 100.0;
const float Z_NEAR = 0.1;
const float Z_RANGE = 100.0 - 0.1;

uniform bool use_linear_depth;

void main() {

    float depth = gl_FragCoord.z;

    if (use_linear_depth) {
        // Re-transform the depth values from [0, 1] to NDC in the range [-1, 1],
        // and reverse the non-linear transform applied by the projection matrix.
        // Reference: http://www.songho.ca/opengl/gl_projectionmatrix.html
        float ndc = 2.0 * gl_FragCoord.z - 1.0;
        float linear_depth = (2.0 * Z_NEAR * Z_FAR) / (Z_FAR + Z_NEAR - ndc * Z_RANGE);

        // Map linear depth from the range [Z_NEAR, Z_FAR] to [~0, 1] to visualize it.
        depth = linear_depth / Z_FAR;
    }

    // Map the linear depth in the range [Z_NEAR, Z_FAR] to [~0, 1].
    fragColor = vec4(vec3(depth), 1.0);
}
