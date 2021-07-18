#version 330 core

out vec4 fragColor;

in vec2 texcoord;

uniform sampler2D texture_diffuse;

void main() {
    vec4 color = texture(texture_diffuse, texcoord);
    // if (color.a < 0.1) { discard; }
    fragColor = color;
}
