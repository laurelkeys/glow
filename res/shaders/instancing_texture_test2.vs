#version 330

layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in mat4 aInstanceLocalToWorld; // model

out vec2 texcoord;

uniform mat4 world_to_view; // view
uniform mat4 view_to_clip; // projection

void main() {
    texcoord = aTexCoord;
    gl_Position = vec4(aPos, 1.0) * aInstanceLocalToWorld * world_to_view * view_to_clip;
}
