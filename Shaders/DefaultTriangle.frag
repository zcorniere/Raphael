#version 450

layout(location = 0) in vec3 inVertex;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(inVertex.xyz, 1.0);
}
