#version 330 core

out float fragColor;

uniform vec3 samples[64];

uniform mat4 view_to_clip; // projection

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

void main() {
    // @Todo: ...
}
