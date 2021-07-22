#version 330

layout (location = 0) in vec3 aPos;
layout (location = 3) in vec2 aOffset; // @Note: mesh.c's `init_mesh_vao` sets locations 0, 1 and 2

out vec3 color;

uniform mat4 local_to_world; // model
uniform mat4 world_to_view; // view
uniform mat4 view_to_clip; // projection

// Reference: https://github.com/lolengine/lolengine/blob/master/COPYING
vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
    color = hsv2rgb(vec3(float(gl_InstanceID) / 99.0, 1.0, 1.0));
    gl_Position = vec4(aPos + vec3(aOffset, -10.0), 1.0) * local_to_world * world_to_view * view_to_clip;
}
