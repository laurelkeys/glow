#version 330 core

layout (location = 0) in vec3 aPos;

out vec3 tex_coords;

uniform mat4 world_to_view; // view
uniform mat4 view_to_clip; // projection

void main() {
    tex_coords = aPos;
    vec4 pos = vec4(aPos, 1.0) * world_to_view * view_to_clip;
    gl_Position = pos.xyww;
}
