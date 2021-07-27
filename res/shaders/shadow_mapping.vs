#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out VS_OUT {
    vec3 frag_pos;
    vec3 normal;
    vec2 texcoord;
    vec4 frag_pos_light_space;
} vs_out;

uniform mat4 local_to_world; // model
uniform mat4 world_to_view; // view
uniform mat4 view_to_clip; // projection
uniform mat4 world_to_light_space; // light space (viewprojection) matrix

void main() {
    // Transform the world-space vertex position to light space,
    // to use it for shadow mapping in the fragment shader.
    vec4 frag_pos_world = vec4(aPos, 1.0) * local_to_world;
    vs_out.frag_pos_light_space = frag_pos_world * world_to_light_space;

    vs_out.frag_pos = vec3(frag_pos_world);
    vs_out.normal = aNormal * transpose(inverse(mat3(local_to_world)));
    vs_out.texcoord = aTexCoord;

    gl_Position = vec4(vs_out.frag_pos, 1.0) * world_to_view * view_to_clip;
}
