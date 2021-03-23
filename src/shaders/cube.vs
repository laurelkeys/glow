#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 tex_coords;
out vec3 light_in_view;
out vec3 frag_in_view;
out vec3 frag_normal;

uniform vec3 light_in_world;

uniform mat4 local_to_world; // model
uniform mat4 world_to_view; // view
uniform mat4 view_to_clip; // projection

void main() {
    mat4 local_to_view = local_to_world * world_to_view;

    tex_coords = aTexCoords;
    light_in_view = vec3(vec4(light_in_world, 1.0) * world_to_view);
    frag_in_view = vec3(vec4(aPos, 1.0) * local_to_view);
    frag_normal = aNormal * mat3(transpose(inverse(local_to_view)));

    gl_Position = vec4(frag_in_view, 1.0) * view_to_clip;
}
