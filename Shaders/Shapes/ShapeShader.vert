#version 460

layout(location = 0) in vec3 inVertex;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outNormal;
;

void main()
{
    outPosition = inVertex;
    outNormal = inNormal;

    gl_Position = vec4(inVertex, 1.0);
}
