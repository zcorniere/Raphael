#pragma once

#include "Engine/Containers/ResourceArray.hxx"
#include "Engine/Core/RHI/Resources/RHIBuffer.hxx"

namespace Shapes
{

struct FVertex {
    FVector3 Position;
    FVector3 Normal;
};

struct FShape {
    TResourceArray<FVertex> Vertices;
    TResourceArray<uint32> Indices;
};

inline FShape CreateBox(const FVector3& size)
{
    TResourceArray<FVertex> vertices;
    vertices.Resize(8);
    vertices[0].Position = {-size.x / 2.0f, -size.y / 2.0f, size.z / 2.0f};
    vertices[1].Position = {size.x / 2.0f, -size.y / 2.0f, size.z / 2.0f};
    vertices[2].Position = {size.x / 2.0f, size.y / 2.0f, size.z / 2.0f};
    vertices[3].Position = {-size.x / 2.0f, size.y / 2.0f, size.z / 2.0f};
    vertices[4].Position = {-size.x / 2.0f, -size.y / 2.0f, -size.z / 2.0f};
    vertices[5].Position = {size.x / 2.0f, -size.y / 2.0f, -size.z / 2.0f};
    vertices[6].Position = {size.x / 2.0f, size.y / 2.0f, -size.z / 2.0f};
    vertices[7].Position = {-size.x / 2.0f, size.y / 2.0f, -size.z / 2.0f};

    vertices[0].Normal = {-1.0f, -1.0f, 1.0f};
    vertices[1].Normal = {1.0f, -1.0f, 1.0f};
    vertices[2].Normal = {1.0f, 1.0f, 1.0f};
    vertices[3].Normal = {-1.0f, 1.0f, 1.0f};
    vertices[4].Normal = {-1.0f, -1.0f, -1.0f};
    vertices[5].Normal = {1.0f, -1.0f, -1.0f};
    vertices[6].Normal = {1.0f, 1.0f, -1.0f};
    vertices[7].Normal = {-1.0f, 1.0f, -1.0f};

    TResourceArray<uint32> indices{0, 1, 2, 2, 3, 0, 1, 5, 6, 6, 2, 1, 7, 6, 5, 5, 4, 7,
                                   4, 0, 3, 3, 7, 4, 4, 5, 1, 1, 0, 4, 3, 2, 6, 6, 7, 3};

    return FShape{vertices, indices};
}
}    // namespace Shapes
