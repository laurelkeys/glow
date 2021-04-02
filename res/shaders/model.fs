#version 330 core

out vec4 fragColor;

in vec2 tex_coords;

uniform sampler2D texture_diffuse0;

void main() {
    fragColor = texture(texture_diffuse0, tex_coords);
}
