#include "Engine/Core/RHI/RHIDefinitions.hxx"

#include "Engine/Core/RHI/Resources/RHITexture.hxx"

#include <glm/gtx/hash.hpp>

size_t std::hash<RHIRenderPassDescription>::operator()(const RHIRenderPassDescription& Desc) const
{
    size_t Result = 0;

    for (const RHIRenderPassTarget& Target: Desc.ColorTarget) {
        ::Raphael::HashCombine(Result, Target.Format);
    }
    for (const RHIRenderPassTarget& Target: Desc.ResolveTarget) {
        ::Raphael::HashCombine(Result, Target.Format);
    }
    if (Desc.DepthTarget) {
        ::Raphael::HashCombine(Result, Desc.DepthTarget.value().Format);
    }
    return Result;
}

size_t std::hash<RHIFramebufferDefinition>::operator()(const RHIFramebufferDefinition& Desc) const
{
    size_t Result = 0;

    ::Raphael::HashCombine(Result, Desc.Offset);
    for (const Ref<RHITexture>& Target: Desc.ColorTarget) {
        ::Raphael::HashCombine(Result, std::hash<int>()(static_cast<int>(Target->GetDescription().Format)));
        ::Raphael::HashCombine(Result, std::hash<glm::uvec2>()(Target->GetDescription().Extent));
    }
    for (const Ref<RHITexture>& Target: Desc.ResolveTarget) {
        ::Raphael::HashCombine(Result, std::hash<int>()(static_cast<int>(Target->GetDescription().Format)));
        ::Raphael::HashCombine(Result, std::hash<glm::uvec2>()(Target->GetDescription().Extent));
    }
    if (Desc.DepthTarget) {
        ::Raphael::HashCombine(Result, std::hash<int>()(static_cast<int>(Desc.DepthTarget->GetDescription().Format)));
        ::Raphael::HashCombine(Result, std::hash<glm::uvec2>()(Desc.DepthTarget->GetDescription().Extent));
    }
    return Result;
}
