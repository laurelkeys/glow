#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out VS_OUT {
    vec3 normal;
} vs_out;

uniform mat4 local_to_world; // model
uniform mat4 world_to_view; // view

void main() {
    gl_Position = vec4(aPos, 1.0) * local_to_world * world_to_view;
    mat3 normal_matrix = mat3(transpose(inverse(local_to_world * world_to_view)));
    vs_out.normal = normalize(aNormal * normal_matrix);
}
