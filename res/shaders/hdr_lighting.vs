#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out VS_OUT {
    vec3 frag_pos;
    vec3 normal;
    vec2 texcoord;
} vs_out;

uniform mat4 local_to_world; // model
uniform mat4 world_to_view; // view
uniform mat4 view_to_clip; // projection

uniform bool invert_normals;

void main() {
    vec4 pos_world = vec4(aPos, 1.0) * local_to_world;
    vec3 n = invert_normals ? -aNormal : aNormal;
    mat3 normal_matrix = transpose(inverse(mat3(local_to_world)));

    vs_out.frag_pos = vec3(pos_world);
    vs_out.normal = normalize(n * normal_matrix);
    vs_out.texcoord = aTexCoord;

    gl_Position = pos_world * world_to_view * view_to_clip;
}
