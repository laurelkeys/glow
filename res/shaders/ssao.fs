#version 330 core

out float fragColor;

in vec2 texcoord;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform mat4 view_to_clip; // projection

uniform vec2 noise_scale;

uniform float radius; // 0.5
uniform float bias; // 0.025

uniform vec3 samples[64];
int samples_count = 64;

void main() {
    vec3 frag_pos = texture(gPosition, texcoord).xyz;
    vec3 normal = texture(gNormal, texcoord).xyz;
    vec3 random = texture(texNoise, texcoord * noise_scale).xyz;

    vec3 tangent = normalize(random - normal * dot(random, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 tbn_matrix = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    for (int i = 0; i < samples_count; ++i) {
        // Sample position, transforming it from tangent to view-space.
        vec3 samples_pos = tbn_matrix * samples[i];
        samples_pos = frag_pos + samples_pos * radius;

        // Project it from view-space to screen-space.
        vec4 offset = vec4(samples_pos, 1.0);
        offset = offset * view_to_clip;
        offset.xyz /= offset.w; // perspective divide (NDC)
        offset.xyz = offset.xyz * 0.5 + 0.5; // [-1, 1] -> [0, 1]

        float sample_depth = texture(gPosition, offset.xy).z;
        // float range_check = min(1.0, radius / abs(frag_pos.z - sample_depth));
        float range_check = smoothstep(0.0, 1.0, radius / abs(frag_pos.z - sample_depth));
        occlusion += ((sample_depth >= sample_pos.z + bias) ? 1.0 : 0.0) * range_check;
    }

    // Normalize the occlusion factor and save it subtracted from 1.0, so that
    // we can use the output directly to scale the ambient lighting component.
    occlusion = occlusion / samples_count;
    fragColor = 1.0 - occlusion;
}
