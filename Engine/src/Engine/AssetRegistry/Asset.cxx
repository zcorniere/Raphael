#include "Engine/AssetRegistry/Asset.hxx"

#include "Engine/Core/RHI/RHI.hxx"

RAsset::RAsset(const std::filesystem::path& Path): bIsMemoryOnly(false), AssetPath(Path.string())
{
    SetName(Path.stem().string());
}

RAsset::RAsset(const TResourceArray<FVertex>& Vertices, const TResourceArray<uint32>& Indices): bIsMemoryOnly(true)
{
    VertexData = Vertices;
    IndexData = Indices;
}

RAsset::~RAsset()
{
    Unload();
}

bool RAsset::Load()
{
    if (bIsMemoryOnly) {
        return false;
    }
    return true;
}

bool RAsset::LoadOnGPU()
{
    if (IsLoadedOnGPU()) {
        return true;
    }
    Ref<RRHIBuffer> TmpBuffer = RHI::CreateBuffer(FRHIBufferDesc{
        .Size = VertexData.GetByteSize(),
        .Stride = sizeof(FVertex),
        .Usage = EBufferUsageFlags::SourceCopy | EBufferUsageFlags::KeepCPUAccessible,
        .ResourceArray = &VertexData,
        .DebugName = std::format("{:s}.StagingVertexBuffer", GetName()),
    });
    Ref<RRHIBuffer> TmpIndexBuffer = RHI::CreateBuffer(FRHIBufferDesc{
        .Size = IndexData.GetByteSize(),
        .Stride = sizeof(uint32),
        .Usage = EBufferUsageFlags::SourceCopy | EBufferUsageFlags::KeepCPUAccessible,
        .ResourceArray = &IndexData,
        .DebugName = std::format("{:s}.StagingIndexBuffer", GetName()),
    });

    ENQUEUE_RENDER_COMMAND(CopyBuffer)
    ([this, TmpBuffer, TmpIndexBuffer](FFRHICommandList& CommandList) {
        VertexBuffer = RHI::CreateBuffer(FRHIBufferDesc{
            .Size = TmpBuffer->GetSize(),
            .Stride = TmpBuffer->GetStride(),
            .Usage = EBufferUsageFlags::VertexBuffer | EBufferUsageFlags::DestinationCopy,
            .ResourceArray = nullptr,
            .DebugName = std::format("{:s}.VertexBuffer", GetName()),
        });
        IndexBuffer = RHI::CreateBuffer(FRHIBufferDesc{
            .Size = TmpIndexBuffer->GetSize(),
            .Stride = TmpIndexBuffer->GetStride(),
            .Usage = EBufferUsageFlags::IndexBuffer | EBufferUsageFlags::DestinationCopy,
            .ResourceArray = nullptr,
            .DebugName = std::format("{:s}.IndexBuffer", GetName()),
        });
        CommandList.CopyBufferToBuffer(TmpBuffer, VertexBuffer, 0, 0, TmpBuffer->GetSize());
        CommandList.CopyBufferToBuffer(TmpIndexBuffer, IndexBuffer, 0, 0, TmpIndexBuffer->GetSize());

        RHI::RHIWaitUntilIdle();
    });

    return true;
}

void RAsset::Unload()
{
    VertexBuffer = nullptr;
    IndexBuffer = nullptr;

    VertexData.Clear();
    IndexData.Clear();
}
