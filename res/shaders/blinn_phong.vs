#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 frag_pos;
out vec3 frag_normal;
out vec2 texcoord;

uniform mat4 local_to_world; // model
uniform mat4 world_to_view; // view
uniform mat4 view_to_clip; // projection

void main() {
    frag_pos = vec3(vec4(aPos, 1.0) * local_to_world);
    frag_normal = aNormal * mat3(transpose(inverse(local_to_world)));
    texcoord = aTexCoord;

    gl_Position = vec4(frag_pos, 1.0) * world_to_view * view_to_clip;
}
