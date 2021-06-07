#version 330 core

out vec4 fragColor;

#define PHONG_SHININESS_MULTIPLIER 2.0
#define POINT_LIGHTS_LEN 4

struct Components { vec3 ambient; vec3 diffuse; vec3 specular; };
struct Attenuation { float constant; float linear; float quadratic; };

struct DirectionalLight {
    vec3 direction;
    Components k;
};
struct PointLight {
    vec3 position;
    Components k;
    Attenuation att;
};
struct SpotLight {
    vec3 position;
    vec3 direction;
    float cos_cutoff_inner;
    float cos_cutoff_outer;
    Components k;
    Attenuation att;
};

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

in vec3 frag_pos;
in vec3 frag_normal;
in vec2 texcoord;

uniform bool use_blinn_phong;
uniform vec3 view_pos;
uniform DirectionalLight directional_light;
uniform PointLight point_lights[POINT_LIGHTS_LEN];
uniform SpotLight spot_light;
uniform Material material;

// Forward declarations.
vec3 compute_directional_light(DirectionalLight light, vec3 normal, vec3 view_dir);
vec3 compute_point_light(PointLight light, vec3 normal, vec3 view_dir, vec3 frag_pos);
vec3 compute_spot_light(SpotLight light, vec3 normal, vec3 view_dir, vec3 frag_pos);

void main() {
    vec3 normal = normalize(frag_normal);
    vec3 view_dir = normalize(view_pos - frag_pos);

    vec3 result = compute_directional_light(directional_light, normal, view_dir);
    for (int i = 0; i < POINT_LIGHTS_LEN; ++i) {
        result += compute_point_light(point_lights[i], normal, view_dir, frag_pos);
    }
    result += compute_spot_light(spot_light, normal, view_dir, frag_pos);

    fragColor = vec4(result, 1.0);
}

vec3 light_color(Components light, vec3 light_dir, vec3 normal, vec3 view_dir) {
    vec3 ambient = light.ambient * texture(material.diffuse, texcoord).rgb;

    float diff = max(dot(normal, light_dir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, texcoord).rgb;

    float spec = 0.0;
    if (use_blinn_phong) {
        vec3 halfway_dir = normalize(light_dir + view_dir);
        spec = pow(max(dot(normal, halfway_dir), 0.0), PHONG_SHININESS_MULTIPLIER * material.shininess);
    } else {
        vec3 reflect_dir = reflect(-light_dir, normal);
        spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    }
    vec3 specular = light.specular * spec * texture(material.specular, texcoord).rgb;

    return ambient + diffuse + specular;
}

float light_attenuation(Attenuation att, float dist) {
    return 1.0 / (att.constant + att.linear * dist + att.quadratic * (dist * dist));
}

// Directional light.
vec3 compute_directional_light(DirectionalLight light, vec3 normal, vec3 view_dir) {
    vec3 light_dir = normalize(-light.direction);
    return light_color(light.k, light_dir, normal, view_dir);
}

// Point light.
vec3 compute_point_light(PointLight light, vec3 normal, vec3 view_dir, vec3 frag_pos) {
    vec3 light_dir = normalize(light.position - frag_pos);
    vec3 color = light_color(light.k, light_dir, normal, view_dir);

    float dist = length(light.position - frag_pos);
    float attenuation = light_attenuation(light.att, dist);

    return attenuation * color;
}

// Spot light.
vec3 compute_spot_light(SpotLight light, vec3 normal, vec3 view_dir, vec3 frag_pos) {
    vec3 light_dir = normalize(light.position - frag_pos);
    vec3 color = light_color(light.k, light_dir, normal, view_dir);

    float dist = length(light.position - frag_pos);
    float attenuation = light_attenuation(light.att, dist);

    float cos_theta = dot(light_dir, normalize(-light.direction));
    float epsilon = light.cos_cutoff_inner - light.cos_cutoff_outer;
    float intensity = clamp((cos_theta - light.cos_cutoff_outer) / epsilon, 0.0, 1.0);

    return intensity * attenuation * color;
}
