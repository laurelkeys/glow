#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 pos;
out vec3 normal;

uniform mat4 local_to_world; // model
uniform mat4 world_to_view; // view
uniform mat4 view_to_clip; // projection

void main() {
    vec4 position_in_world = vec4(aPos, 1.0) * local_to_world;

    pos = vec3(position_in_world);
    normal = aNormal * mat3(transpose(inverse(local_to_world)));

    gl_Position = position_in_world * world_to_view * view_to_clip;
}
