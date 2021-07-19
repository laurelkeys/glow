#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec2 texcoord;
} gs_in[];

out vec2 texcoord;

uniform float time;

vec4 explode(vec4 position, vec3 normal) {
    float magnitude = 2.0;
    vec3 direction = normal * (sin(time) * 0.5 + 0.5) * magnitude;
    return position + vec4(direction, 0.0);
}

vec3 triangle_normal() {
    vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
    vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
    return normalize(cross(a, b));
}

void main() {
    vec3 normal = triangle_normal();

    gl_Position = explode(gl_in[0].gl_Position, normal);
    texcoord = gs_in[0].texcoord;
    EmitVertex();

    gl_Position = explode(gl_in[1].gl_Position, normal);
    texcoord = gs_in[1].texcoord;
    EmitVertex();

    gl_Position = explode(gl_in[2].gl_Position, normal);
    texcoord = gs_in[2].texcoord;
    EmitVertex();

    EndPrimitive();
}
