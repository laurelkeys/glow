#version 330 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in VS_OUT {
    vec3 frag_pos;
    vec3 normal;
    vec2 texcoord;
} fs_in;

uniform sampler2D texture_diffuse;
uniform sampler2D texture_specular;

void main() {
    gPosition = fs_in.frag_pos;
    gNormal = normalize(fs_in.normal);
    // @Note: we pack both albedo and specular intensity into a single texture.
    gAlbedoSpec.rgb = texture(texture_diffuse, fs_in.texcoord).rgb;
    gAlbedoSpec.a = texture(texture_specular, fs_in.texcoord).r;
}
