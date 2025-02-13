#version 460

#include <include/Buffers.glsl>

#include <include/VertexInput.glsl>

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outNormal;

void main()
{
    mat4 Matrix = GetTransformMatrix();
    outPosition = inPosition;
    outNormal = inNormal;

    vec4 worldPosition = Matrix * vec4(inPosition, 1.0);
    gl_Position = u_Camera.viewproj * worldPosition;
}
