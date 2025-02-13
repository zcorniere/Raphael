#pragma once

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBitangent;
layout(location = 4) in vec2 inTexCoord;

layout(location = 5) in vec4 inTransformMatrixA;
layout(location = 6) in vec4 inTransformMatrixB;
layout(location = 7) in vec4 inTransformMatrixC;
layout(location = 8) in vec4 inTransformMatrixD;

mat4 GetTransformMatrix()
{
    return mat4(inTransformMatrixA, inTransformMatrixB, inTransformMatrixC, inTransformMatrixD);
}
