#version 330 core

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in VS_OUT {
    vec3 normal;
} gs_in[];

const float MAGNITUDE = 0.4;

uniform mat4 view_to_clip; // projection

void generate_line(int index) {
    gl_Position = gl_in[index].gl_Position * view_to_clip;
    EmitVertex();

    vec4 normal = vec4(gs_in[index].normal, 0.0) * MAGNITUDE;
    gl_Position = (gl_in[index].gl_Position + normal) * view_to_clip;
    EmitVertex();

    EndPrimitive();
}

void main() {
    generate_line(0); // 1st vertex normal
    generate_line(1); // 2nd vertex normal
    generate_line(2); // 3rd vertex normal
}
