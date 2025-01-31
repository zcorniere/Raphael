#include "Engine/AssetRegistry/MeshFactory.hxx"

Ref<RAsset> MeshFactory::CreateBox(const FVector3& size)
{
    TResourceArray<RAsset::FVertex> vertices;
    vertices.Resize(8);
    vertices[0].Position = {-1.0f, -1.0f, 1.0f};
    vertices[1].Position = {1.0f, -1.0f, 1.0f};
    vertices[2].Position = {1.0f, 1.0f, 1.0f};
    vertices[3].Position = {-1.0f, 1.0f, 1.0f};
    vertices[4].Position = {-1.0f, -1.0f, -1.0f};
    vertices[5].Position = {1.0f, -1.0f, -1.0f};
    vertices[6].Position = {1.0f, 1.0f, -1.0f};
    vertices[7].Position = {-1.0f, 1.0f, -1.0f};

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

    return Ref<RAsset>::CreateNamed("Box", vertices, indices);
}
