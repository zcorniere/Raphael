#include "Engine/Core/RHI/RHIDefinitions.hxx"

#include "Engine/Core/RHI/Resources/RHITexture.hxx"

size_t std::hash<RHIRenderPassDescription>::operator()(const RHIRenderPassDescription& Desc) const
{
    size_t Result = 0;
    ::Raphael::HashCombine(Result, Desc.Offset);
    ::Raphael::HashCombine(Result, Desc.Size);
    for (const Ref<RHITexture>& Target: Desc.ColorTarget) {
        ::Raphael::HashCombine(Result, Target->GetDescription().Format);
    }
    for (const Ref<RHITexture>& Target: Desc.ResolveTarget) {
        ::Raphael::HashCombine(Result, Target->GetDescription().Format);
    }
    if (Desc.DepthTarget) {
        ::Raphael::HashCombine(Result, Desc.DepthTarget->GetDescription().Format);
    }
    return Result;
}
