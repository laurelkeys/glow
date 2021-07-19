#version 330 core

out vec4 fragColor;

in VS_OUT {
    vec2 texcoord;
} fs_in;

uniform sampler2D texture_diffuse;

void main() {
    vec4 color = texture(texture_diffuse, fs_in.texcoord);
    // if (color.a < 0.1) { discard; }
    fragColor = color;
}
