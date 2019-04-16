#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec4 inColor;
layout (location = 1) in float inDepth;

layout (location = 0) out vec4 outColor;

void main() {
    outColor = inColor;
    if (inDepth < 0.01) {
        discard;
    }
    gl_FragDepth = inDepth;
}