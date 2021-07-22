#version 330 core

out vec4 fragColor;

in vec2 texcoord;

uniform sampler2D texture_diffuse;

void main() {
    fragColor = texture(texture_diffuse, texcoord);
}
