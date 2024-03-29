#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 local_to_world; // model
uniform mat4 world_to_view; // view
uniform mat4 view_to_clip; // projection

void main() {
    gl_Position = vec4(aPos, 1.0) * local_to_world * world_to_view * view_to_clip;
}
