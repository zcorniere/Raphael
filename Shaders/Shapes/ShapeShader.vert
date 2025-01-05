#version 460

layout(location = 0) in vec3 inVertex;
layout(location = 1) in vec3 inNormal;

void main()
{
    gl_Position = vec4(inVertex, 1.0);
}
