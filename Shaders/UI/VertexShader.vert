#version 460

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec4 inColor;

layout(location = 0) out vec2 outPosition;
layout(location = 1) out vec2 outTexCoord;
layout(location = 2) out vec4 outColor;

void main()
{
    vec4 position = vec4(inPosition, 0.0, 1.0);
    gl_Position = position;
    outPosition = inPosition;
    outTexCoord = inTexCoord;
    outColor = inColor;
}
