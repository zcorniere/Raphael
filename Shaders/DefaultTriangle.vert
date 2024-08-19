#version 460

layout(location = 0) in vec3 inVertex;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(inColor, 1.0);
    gl_Position = vec4(inVertex, 1.0);
}
