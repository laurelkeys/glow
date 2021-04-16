#version 330 core

out vec4 fragColor;

in vec2 tex_coords;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

uniform Material material;

void main() {
    fragColor = texture(material.diffuse, tex_coords);
}
