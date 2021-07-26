#version 330 core

// @Note: this shader is meant to be used when there is no color buffer
// and the draw and read buffers have been disabled, so it's just empty.

void main() {
    // This is what effectively happens:
    /* gl_FragDepth = gl_FragCoord.z; */
}
