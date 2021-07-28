#version 330 core

out vec4 fragColor;

in vec2 texcoord;

uniform sampler2D hdr_buffer;
uniform bool tonemap;
uniform float exposure;

void main() {
    const float gamma = 2.2;
    vec3 hdr_color = texture(hdr_buffer, texcoord).rgb;

    vec3 result = hdr_color;
    if (tonemap) {
        // result = hdr_color / (hdr_color + vec3(1.0)); // reinhard
        result = vec3(1.0) - exp(-hdr_color * exposure); // exposure
    }

    fragColor = vec4(pow(result, vec3(1.0 / gamma)), 1.0);
}
