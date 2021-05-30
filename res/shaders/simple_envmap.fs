#version 330 core

out vec4 fragColor;

in vec3 pos;
in vec3 normal;

uniform vec3 camera_pos;
uniform samplerCube skybox;

#define IOR_AIR     1.00
#define IOR_WATER   1.33
#define IOR_ICE     1.309
#define IOR_GLASS   1.52
#define IOR_DIAMOND 2.42

void main() {
    vec3 N = normalize(normal);
    vec3 I = normalize(pos - camera_pos);

    // vec3 R = reflect(I, N);
    // fragColor = vec4(texture(skybox, R).rgb, 1.0);

    vec3 T = refract(I, N, IOR_AIR / IOR_WATER);
    // vec3 T = refract(I, N, IOR_AIR / IOR_ICE);
    // vec3 T = refract(I, N, IOR_AIR / IOR_GLASS);
    // vec3 T = refract(I, N, IOR_AIR / IOR_DIAMOND);
    fragColor = vec4(texture(skybox, T).rgb, 1.0);
}
