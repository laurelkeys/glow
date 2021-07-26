#version 330 core

out vec4 fragColor;

in vec2 texcoord;

uniform sampler2D depth_map;
uniform float near_plane;
uniform float far_plane;

#define USE_ORTHO 1

#if !USE_ORTHO
float linearize_depth(float depth) {
    float z = depth * 2.0 - 1.0; // back to NDC
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}
#endif

void main() {
    float depth_value = texture(depth_map, texcoord).r;
#if USE_ORTHO
    fragColor = vec4(vec3(depth_value), 1.0);
#else
    fragColor = vec4(vec3(linearize_depth(depth_value) / far_plane), 1.0);
#endif
}
