#version 450

layout(location = 0) in vec2 inVertex;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(inVertex.xy, 1.0, 1.0);
}
