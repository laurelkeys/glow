#version 330 core

out vec4 fragColor;

in vec2 tex_coords;

uniform sampler2D texture1;
uniform sampler2D texture2;

void main() {
    fragColor = mix(texture(texture1, tex_coords), texture(texture2, tex_coords), 0.2);
}
