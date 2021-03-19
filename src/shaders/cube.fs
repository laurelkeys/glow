#version 330 core

out vec4 fragColor;

uniform vec3 light_color;
uniform vec3 object_color;

void main() {
    fragColor = vec4(light_color * object_color, 1.0);
}
