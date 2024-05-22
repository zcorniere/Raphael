#version 450

layout(location = 0) in vec2 inVertex;

layout(location = 0) out vec3 outVertex;

void main() {
    outVertex = vec3(inVertex, gl_VertexIndex);
    gl_Position = vec4(inVertex, 0.0, 1.0);
}
