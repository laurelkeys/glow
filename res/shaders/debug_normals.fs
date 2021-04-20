#version 330 core

out vec4 fragColor;

// @Note: uncomment /* flat */ to see face normals.
// @Volatile: this must also be done in the vertex shader.
/* flat */ in vec3 vert_normal;

void main() {
    fragColor = vec4(vert_normal, 1.0);
}
