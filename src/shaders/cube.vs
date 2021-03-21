#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 frag_in_world;
out vec3 normal_in_world;

uniform mat4 local_to_world; // model
uniform mat4 world_to_view; // view
uniform mat4 view_to_clip; // projection

void main() {
    frag_in_world = vec3(vec4(aPos, 1.0) * local_to_world);
    normal_in_world = aNormal * mat3(transpose(inverse(local_to_world)));

    gl_Position = vec4(frag_in_world, 1.0) * world_to_view * view_to_clip;
}
