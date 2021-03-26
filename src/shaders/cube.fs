#version 330 core

out vec4 fragColor;

struct Light {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

in vec2 tex_coords;
in vec3 light_in_view;
in vec3 frag_in_view;
in vec3 frag_normal;

uniform Light light;
uniform Material material;

void main() {
    // Ambient light.
    vec3 ambient = light.ambient * texture(material.diffuse, tex_coords).rgb;

    // Diffuse light.
    vec3 normal = normalize(frag_normal);
    vec3 light_dir = normalize(light_in_view - frag_in_view);
    float diff = max(dot(normal, light_dir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, tex_coords).rgb;

    // Specular light.
    vec3 view_dir = normalize(-frag_in_view);
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * texture(material.specular, tex_coords).rgb;

    fragColor = vec4(ambient + diffuse + specular, 1.0);
}
