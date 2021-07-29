#version 330 core

out vec4 fragColor;

in VS_OUT {
    vec3 frag_pos;
    vec3 normal;
    vec2 texcoord;
} fs_in;

struct Light {
    vec3 position;
    vec3 color;
};

#define LIGHTS_LEN 4

uniform Light lights[LIGHTS_LEN];
uniform sampler2D texture_diffuse;
uniform vec3 view_pos;

void main() {
    vec3 color = texture(texture_diffuse, fs_in.texcoord).rgb;
    vec3 normal = normalize(fs_in.normal);

    // ambient
    vec3 ambient = 0.01 * color;

    // lighting
    vec3 lighting = vec3(0.0);
    for (int i = 0; i < LIGHTS_LEN; ++i) {
        vec3 light_dir = normalize(lights[i].position - fs_in.frag_pos);
        float diff = max(dot(light_dir, normal), 0.0);
        vec3 diffuse = lights[i].color * diff * color;

        // Attenuation (use quadratic as we do gamma correction).
        float dist = length(fs_in.frag_pos - lights[i].position);

        lighting += diffuse * (1.0 / (dist * dist));
    }

    fragColor = vec4(ambient + lighting, 1.0);
}
