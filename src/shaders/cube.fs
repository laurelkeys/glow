#version 330 core

out vec4 fragColor;

in vec3 frag_in_world;
in vec3 normal_in_world;

uniform vec3 light_color;
uniform vec3 object_color;
uniform vec3 light_in_world;
uniform vec3 eye_in_world;

void main() {
    float ambient_strength = 0.1;
    float specular_strength = 0.5;
    int shininess = 32;

    vec3 norm = normalize(normal_in_world);
    vec3 view_dir = normalize(eye_in_world - frag_in_world);
    vec3 light_dir = normalize(light_in_world - frag_in_world);
    // @Note: `reflect` expects the incident vector to point from the
    // light source towards the fragment position, so we negate `light_dir`.
    vec3 reflect_dir = reflect(-light_dir, norm);

    // Compute ambient, diffuse and specular light components.
    vec3 ambient = ambient_strength * light_color;
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * light_color;
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), shininess);
    vec3 specular = specular_strength * spec * light_color;

    fragColor = vec4((ambient + diffuse + specular) * object_color, 1.0);
}
