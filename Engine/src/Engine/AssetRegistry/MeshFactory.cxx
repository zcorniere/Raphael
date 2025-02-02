#include "Engine/AssetRegistry/MeshFactory.hxx"

Ref<RAsset> MeshFactory::CreateBox(const FVector3& size)
{
    TResourceArray<RAsset::FVertex> vertices;
    vertices.Resize(8);
    vertices[0].Position = {-size.x / 2, -size.y / 2, size.z / 2};
    vertices[1].Position = {size.x / 2, -size.y / 2, size.z / 2};
    vertices[2].Position = {size.x / 2, size.y / 2, size.z / 2};
    vertices[3].Position = {-size.x / 2, size.y / 2, size.z / 2};
    vertices[4].Position = {-size.x / 2, -size.y / 2, -size.z / 2};
    vertices[5].Position = {size.x / 2, -size.y / 2, -size.z / 2};
    vertices[6].Position = {size.x / 2, size.y / 2, -size.z / 2};
    vertices[7].Position = {-size.x / 2, size.y / 2, -size.z / 2};

    vertices[0].Normal = {-size.x / 2, -size.y / 2, size.z / 2};
    vertices[1].Normal = {size.x / 2, -size.y / 2, size.z / 2};
    vertices[2].Normal = {size.x / 2, size.y / 2, size.z / 2};
    vertices[3].Normal = {-size.x / 2, size.y / 2, size.z / 2};
    vertices[4].Normal = {-size.x / 2, -size.y / 2, -size.z / 2};
    vertices[5].Normal = {size.x / 2, -size.y / 2, -size.z / 2};
    vertices[6].Normal = {size.x / 2, size.y / 2, -size.z / 2};
    vertices[7].Normal = {-size.x / 2, size.y / 2, -size.z / 2};

    TResourceArray<uint32> indices{0, 1, 2, 2, 3, 0, 1, 5, 6, 6, 2, 1, 7, 6, 5, 5, 4, 7,
                                   4, 0, 3, 3, 7, 4, 4, 5, 1, 1, 0, 4, 3, 2, 6, 6, 7, 3};

    return Ref<RAsset>::CreateNamed("Box", vertices, indices);
}
