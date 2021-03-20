#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 fragPos;
out vec3 normal;

uniform mat4 local_to_world; // model
uniform mat4 world_to_view; // view
uniform mat4 view_to_clip; // projection

void main() {
    fragPos = vec3(vec4(aPos, 1.0) * local_to_world);
    normal = aNormal;

    gl_Position = vec4(fragPos, 1.0) * world_to_view * view_to_clip;
}
