#version 330

layout (location = 0) in vec3 aPos;

out vec3 color;

uniform vec2 offsets[100];

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
    vec3 offset = vec3(offsets[gl_InstanceID], -10.0);
    vec4 position = vec4(aPos+ offset, 1.0) * local_to_world * world_to_view * view_to_clip;
    gl_Position = position ;
}
