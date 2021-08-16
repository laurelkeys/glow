#version 330 core

out vec4 fragColor;

struct Light {
    vec3 position;
    vec3 color;

    // Light attenuation factors.
    float constant;
    float linear;
    float quadratic;

    // Light volume effect radius.
    float radius;
};

in vec2 texcoord;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

#define LIGHT_COUNT 32
uniform Light lights[LIGHT_COUNT];

uniform vec3 view_pos;

uniform int draw_mode;
#define DRAW_POSITION 1
#define DRAW_NORMAL   2
#define DRAW_ALBEDO   3
#define DRAW_SPECULAR 4

void main() {
    vec3 frag_pos = texture(gPosition, texcoord).rgb;
    vec3 normal = texture(gNormal, texcoord).rgb;
    vec3 diffuse = texture(gAlbedoSpec, texcoord).rgb;
    float specular = texture(gAlbedoSpec, texcoord).a;

    // Debug the intermediate g-buffer textures.
    if (0 < draw_mode && draw_mode < 5) {
        if (draw_mode == DRAW_POSITION) fragColor = vec4(frag_pos,       1.0);
        if (draw_mode == DRAW_NORMAL)   fragColor = vec4(normal,         1.0);
        if (draw_mode == DRAW_ALBEDO)   fragColor = vec4(diffuse,        1.0);
        if (draw_mode == DRAW_SPECULAR) fragColor = vec4(vec3(specular), 1.0);
        return;
    }

    vec3 ambient = diffuse * 0.1;
    vec3 lighting = ambient;
    vec3 view_dir = normalize(view_pos - frag_pos);

    for (int i = 0; i < LIGHT_COUNT; ++i) {
        float dist = length(lights[i].position - frag_pos);
        if (dist < lights[i].radius) {
            vec3 light_dir = normalize(lights[i].position - frag_pos);
            vec3 diffuse = max(dot(normal, light_dir), 0.0) * diffuse * lights[i].color;

            vec3 halfway_dir = normalize(light_dir + view_dir);
            float spec = pow(max(dot(normal, halfway_dir), 0.0), 16.0);
            vec3 specular = lights[i].color * spec * specular;

            float attenuation = 1.0 / (lights[i].constant + lights[i].linear * dist + lights[i].quadratic * dist * dist);

            lighting += attenuation * (diffuse + specular);
        } else {
            // Do nothing, as the fragment is outside of the light's volume.
        }
    }

    fragColor = vec4(lighting, 1.0);
}
