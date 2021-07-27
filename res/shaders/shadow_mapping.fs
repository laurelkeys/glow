#version 330 core

out vec4 fragColor;

in VS_OUT {
    vec3 frag_pos;
    vec3 normal;
    vec2 texcoord;
    vec4 frag_pos_light_space;
} fs_in;

uniform sampler2D texture_diffuse;
uniform sampler2D shadow_map; // ranges [0.0, 1.0]

uniform vec3 light_pos;
uniform vec3 view_pos;

#define PCF 1 // percentage-closer filtering

float shadowing(vec4 frag_pos_light_space) {
    // @Note: when we output a clip-space vertex position to gl_Position in the vertex shader,
    // OpenGL automatically does a perspective divide (transforming clip-space coordinates to
    // normalized device coordinates). As the clip-space frag_pos_light_space isn't passed to
    // the fragment shader through gl_Position, we have to do it ourselves.

    // Perform perspective divide and then map it from [-1.0, 1.0] to [0.0, 1.0] range.
    vec3 proj_coords = (frag_pos_light_space.xyz / frag_pos_light_space.w) * 0.5 + 0.5;

    // Keep the shadow at 0 when it is behind the light frustum's far plane region.
    if (proj_coords.z > 1.0) { return 0.0; }

    // Get the closest and current fragment's depth values from the light's perspective.
    float closest_depth = texture(shadow_map, proj_coords.xy).r;
    float current_depth = proj_coords.z;

    // Calculate shadow bias (based on depth map resolution and slope).
    vec3 normal = normalize(fs_in.normal);
    vec3 light_dir = normalize(light_pos - fs_in.frag_pos);
    float bias = max(0.05 * (1.0 - dot(normal, light_dir)), 0.005);

    // Check whether the current fragment is in shadow
#if !PCF
    float shadow = ((current_depth - bias) > closest_depth) ? 1.0 : 0.0;
#else
    float shadow = 0.0;
    vec2 texel_size = 1.0 / textureSize(/*sampler*/ shadow_map, /*lod*/ 0);
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcf_depth = texture(shadow_map, proj_coords.xy + vec2(x, y) * texel_size).r;
            shadow += ((current_depth - bias) > pcf_depth) ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
#endif

    return shadow;
}

void main() {
    vec3 color = texture(texture_diffuse, fs_in.texcoord).rgb;
    vec3 normal = normalize(fs_in.normal);
    vec3 light_color = vec3(0.3);

    float amb = 0.3;
    vec3 ambient = amb * color;

    vec3 light_dir = normalize(light_pos - fs_in.frag_pos);
    float diff = max(dot(light_dir, normal), 0.0);
    vec3 diffuse = diff * light_color;

    vec3 view_dir = normalize(view_pos - fs_in.frag_pos);
    vec3 reflect_dir = reflect(-light_dir, normal);
    vec3 halfway_dir = normalize(light_dir + view_dir);
    float spec = pow(max(dot(normal, halfway_dir), 0.0), 64.0);
    vec3 specular = spec * light_color;

    // Shadow calculation.
    float shadow = shadowing(fs_in.frag_pos_light_space);

    fragColor = vec4((ambient + (1.0 - shadow) * (diffuse + specular)) * color, 1.0);
}
