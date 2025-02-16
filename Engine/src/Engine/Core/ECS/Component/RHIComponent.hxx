#pragma once

#include "Engine/Core/RHI/Resources/RHITexture.hxx"

namespace ecs
{

class FRenderTargetComponent
{
    RTTI_DECLARE_TYPEINFO_MINIMAL(FRenderTargetComponent);

public:
    Ref<RRHIViewport> Viewport = nullptr;

    TArray<FRHIRenderTarget> ColorTargets = {};
    std::optional<FRHIRenderTarget> DepthTarget = std::nullopt;
    UVector2 Size = {0, 0};

    bool operator==(const FRenderTargetComponent& Other) const = default;
};

}    // namespace ecs
