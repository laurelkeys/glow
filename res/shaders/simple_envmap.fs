#version 330 core

out vec4 fragColor;

in vec3 pos;
in vec3 normal;

uniform vec3 camera_pos;
uniform samplerCube skybox;

void main() {
    vec3 I = normalize(pos - camera_pos);
    vec3 R = reflect(I, normalize(normal));
    fragColor = vec4(texture(skybox, R).rgb, 1.0);
}
