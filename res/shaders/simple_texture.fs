#version 330 core

out vec4 fragColor;

in vec2 tex_coords;

uniform sampler2D texture1;

void main() {
    vec4 color = texture(texture1, tex_coords);
    // if (color.a < 0.1) { discard; }
    fragColor = color;
}
