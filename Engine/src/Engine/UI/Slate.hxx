#pragma once

#include "Engine/Containers/ResourceArray.hxx"
#include "Engine/Core/RHI/Resources/RHIGraphicsPipeline.hxx"

class RSlate : public RObject
{
private:
    BEGIN_UNALIGNED_PARAMETER_STRUCT(FUIVertex)
    PARAMETER(FVector2, Position)
    PARAMETER(FVector2, UV)
    PARAMETER(FVector4, Color)
    END_PARAMETER_STRUCT();

public:
    RSlate() = delete;
    RSlate(Ref<RRHIViewport> InTargetViewport);
    virtual ~RSlate();

    void Rect(float X, float Y, float Width, float Height, FVector4 Color);

    void Draw();

private:
    WeakRef<RRHIViewport> TargetViewport = nullptr;
    Ref<RRHIGraphicsPipeline> GraphicsPipeline = nullptr;

    unsigned DrawCount = 0;
    TResourceArray<FUIVertex> UIVertex;
    Ref<RRHIBuffer> VertexBuffer;
    TResourceArray<uint32> UIIndex;
    Ref<RRHIBuffer> IndexBuffer;
};
