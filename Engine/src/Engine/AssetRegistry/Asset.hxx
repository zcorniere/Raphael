#pragma once

#include "Engine/Containers/ResourceArray.hxx"
#include "Engine/Core/RHI/Resources/RHIBuffer.hxx"

class RAsset : public RObject
{
    RTTI_DECLARE_TYPEINFO(RAsset, RObject);

public:
    struct FVertex {
        FVector3 Position;
        FVector3 Normal;
    };

    struct FDrawInfo {
        uint32 NumVertices;
        uint32 NumIndices;
        uint32 NumPrimitives;
    };

public:
    RAsset(const std::filesystem::path& Path);
    RAsset(const TResourceArray<FVertex>& Vertices, const TResourceArray<uint32>& Indices);
    ~RAsset();

    bool Load();
    bool LoadOnGPU();
    void Unload();
    void UnloadFromGPU();

    bool IsLoaded() const
    {
        return VertexData.IsEmpty() && IndexData.IsEmpty();
    }

    bool IsLoadedOnGPU() const
    {
        return VertexBuffer != nullptr && IndexBuffer != nullptr;
    }

    const Ref<RRHIBuffer> GetVertexBuffer() const
    {
        return VertexBuffer;
    }

    const Ref<RRHIBuffer> GetIndexBuffer() const
    {
        return IndexBuffer;
    }

    FDrawInfo GetDrawInfo() const
    {
        return {VertexData.Size(), IndexData.Size(), IndexData.Size()};
    }

private:
    bool bIsMemoryOnly = false;
    std::string AssetPath;

    Ref<RRHIBuffer> VertexBuffer = nullptr;
    Ref<RRHIBuffer> IndexBuffer = nullptr;

    TResourceArray<FVertex> VertexData;
    TResourceArray<uint32> IndexData;
};
