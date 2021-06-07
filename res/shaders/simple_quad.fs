#version 330 core

out vec4 fragColor;

in vec2 texcoord;

uniform sampler2D screen_texture;

void main() {
    fragColor = texture(screen_texture, texcoord);
}
