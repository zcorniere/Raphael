#version 450

layout(location = 0) in vec4 inVertex;

layout(location = 0) out vec4 outVertexPos;
layout(location = 1) out uvec2 outUV;

void main() {
    outVertexPos = inVertex;
    outUV = uvec2(0, 0);
}
