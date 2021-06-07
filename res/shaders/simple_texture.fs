#version 330 core

out vec4 fragColor;

in vec2 texcoord;

uniform sampler2D texture1;

void main() {
    vec4 color = texture(texture1, texcoord);
    // if (color.a < 0.1) { discard; }
    fragColor = color;
}
