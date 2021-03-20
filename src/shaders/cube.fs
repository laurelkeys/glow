#version 330 core

out vec4 fragColor;

in vec3 fragPos;
in vec3 normal;

uniform vec3 light_pos;
uniform vec3 light_color;
uniform vec3 object_color;

void main() {
    // Ambient light.
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * light_color;

    // Diffuse light.
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(light_pos - fragPos);
    vec3 diffuse = max(dot(norm, lightDir), 0.0) * light_color;

    fragColor = vec4((ambient + diffuse) * object_color, 1.0);
}
