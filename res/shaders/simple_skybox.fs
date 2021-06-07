#version 330 core

out vec4 fragColor;

in vec3 texcoord;

uniform samplerCube skybox;

void main() {
    fragColor = texture(skybox, texcoord);
}
