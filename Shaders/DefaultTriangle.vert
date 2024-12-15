#version 460

layout(location = 0) in vec3 inVertex;

void main()
{
    gl_Position = vec4(inVertex, 1.0);
}
