#version 330 core

layout (location = 0) in vec3 aPos;

out vec3 texcoord;

uniform mat4 world_to_view; // view
uniform mat4 view_to_clip; // projection

void main() {
    texcoord = aPos;
    vec4 pos = vec4(aPos, 1.0) * world_to_view * view_to_clip;

    // @Note: ensure the NDC z-value is 1.0 (maximum depth value).
    gl_Position = pos.xyww;
}
