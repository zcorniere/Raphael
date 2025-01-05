#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec4 outColor;

void main()
{
    vec3 lightPos = vec3(1.2, 1.0, 2.0);

    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * vec3(1.0);

    // diffuse
    vec3 norm = normalize(inNormal);
    vec3 lightDir = normalize(lightPos - inPosition);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0);

    vec3 result = (ambient + diffuse) * 1.0;
    outColor = vec4(result, 1.0);
}
