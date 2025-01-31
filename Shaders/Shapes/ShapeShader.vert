#version 460

layout(location = 0) in vec3 inVertex;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outNormal;

layout(push_constant) uniform readonly constants
{
    mat4 viewproj;
}
CameraData;

void main()
{
    outPosition = inVertex;
    outNormal = inNormal;

    gl_Position = CameraData.viewproj * vec4(inVertex, 1.0);
}
