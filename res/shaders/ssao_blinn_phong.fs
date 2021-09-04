#version 330 core

out vec4 fragColor;

struct Attenuation { float constant; float linear; float quadratic; };

struct Light {
    vec3 position;
    vec3 color;
    float radius;
    Attenuation att;
};

in vec2 texcoord;

uniform Light light;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D ssaoBlurOutput;

void main() {
    vec3 frag_pos = texture(gPosition, texcoord).rgb;
    vec3 normal = texture(gNormal, texcoord).rgb;

    // Blinn-Phong (in view-space).
    vec3 view_dir = normalize(-frag_pos); // @Note: view_pos is (0, 0, 0) in view-space
    vec3 light_dir = normalize(light.position - frag_pos);
    vec3 halfway_dir = normalize(light_dir + view_dir);

    float amb = 0.3;
    float ambient_occlusion = texture(ssaoBlurOutput, texcoord).r;
    vec3 ambient = vec3(amb * diff * ambient_occlusion);

    vec3 diff = texture(gAlbedo, texcoord).rgb;
    vec3 diffuse = max(dot(normal, light_dir), 0.0) * diff * light.color;

    float spec = pow(max(dot(normal, halfway_dir), 0.0), 8.0);
    vec3 specular = spec * light.color;

    // Attenuation.
    float dist = length(light.position - frag_pos);
    float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);

    fragColor = vec4((ambient + diffuse + specular) * attenuation, 1.0);
}
