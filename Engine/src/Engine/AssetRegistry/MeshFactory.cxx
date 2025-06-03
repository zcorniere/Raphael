#include "Engine/AssetRegistry/MeshFactory.hxx"

#include <numbers>

Ref<RAsset> MeshFactory::CreateBox(const FVector3& size)
{
    TResourceArray<FVertex> vertices;
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

static void CalculateRing(size_t segments, float radius, float z, float dz, float height, float actualRadius,
                          TArray<FVertex>& vertices)
{
    float segIncr = 1.0f / (float)(segments - 1);
    for (size_t s = 0; s < segments; s++)
    {
        float x = std::cos(std::numbers::pi_v<float> * 2 * s * segIncr) * radius;
        float y = std::sin(std::numbers::pi_v<float> * 2 * s * segIncr) * radius;

        FVertex& vertex = vertices.Emplace();
        vertex.Position = FVector3(actualRadius * x, actualRadius * y, actualRadius * z + height * dz);
        vertex.Normal = FVector3(x, y, z);
    }
}

// Create a capsule mesh with the given radius and height.
Ref<RAsset> MeshFactory::CreateCapsule(float radius, float height)
{
    const float subdivisionsHeight = 8.f;
    const float ringsBody = subdivisionsHeight + 1.f;
    const float ringsTotal = subdivisionsHeight + ringsBody;
    const float numSegments = 12.f;
    const float radiusModifier = 0.021f;    // Needed to ensure that the wireframe is always visible

    TResourceArray<FVertex> vertices;
    TResourceArray<uint32> indices;

    vertices.Reserve(numSegments * ringsTotal);
    indices.Reserve((numSegments - 1) * (ringsTotal - 1) * 6);    // Adjusted to account for triangles

    float bodyIncr = 1.0f / (ringsBody - 1);
    float ringIncr = 1.0f / (subdivisionsHeight - 1);

    // Calculate vertices for the top hemisphere of the capsule
    for (unsigned r = 0; r < subdivisionsHeight / 2; r++)
        CalculateRing(numSegments, std::sin(std::numbers::pi_v<float> * r * ringIncr),
                      std::sin(std::numbers::pi_v<float> * (r * ringIncr - 0.5f)), -0.5f, height,
                      radius + radiusModifier, vertices);

    // Calculate vertices for the cylindrical body of the capsule
    for (unsigned r = 0; r < ringsBody; r++)
        CalculateRing(numSegments, 1.0f, 0.0f, r * bodyIncr - 0.5f, height, radius + radiusModifier, vertices);

    // Calculate vertices for the bottom hemisphere of the capsule
    for (unsigned r = subdivisionsHeight / 2; r < subdivisionsHeight; r++)
        CalculateRing(numSegments, std::sin(std::numbers::pi_v<float> * r * ringIncr),
                      std::sin(std::numbers::pi_v<float> * (r * ringIncr - 0.5f)), 0.5f, height,
                      radius + radiusModifier, vertices);

    // Generate indices for the capsule mesh
    for (unsigned r = 0; r < ringsTotal - 1; r++)
    {
        for (unsigned s = 0; s < numSegments - 1; s++)
        {
            indices.Add(r * numSegments + s);
            indices.Add((r + 1) * numSegments + s);
            indices.Add(r * numSegments + s + 1);

            indices.Add(r * numSegments + s + 1);
            indices.Add((r + 1) * numSegments + s);
            indices.Add((r + 1) * numSegments + s + 1);
        }
    }

    return Ref<RAsset>::CreateNamed("Capsule", vertices, indices);
}
