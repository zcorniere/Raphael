#version 460

#include <include/Buffers.glsl>

layout(location = 0) in vec3 inVertex;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outNormal;

void main()
{
    outPosition = inVertex;
    outNormal = inNormal;

    gl_Position = u_Camera.viewproj * vec4(inVertex, 1.0);
}
