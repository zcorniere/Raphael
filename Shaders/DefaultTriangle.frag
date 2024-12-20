#version 450

layout(location = 0) out vec4 outColor;

layout(std140, set = 0, binding = 1) uniform ColorValue
{
    vec4 colorValue;
};

void main()
{
    outColor = colorValue;
}
