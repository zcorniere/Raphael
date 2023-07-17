#pragma once

#include "Engine/Core/RHI/RHIDefinitions.hxx"
#include "VulkanRHI/RenderPass/VulkanRenderPass.hxx"

namespace VulkanRHI
{
class VulkanDevice;

class RenderPassManager
{
private:
    struct RenderPassDescriptionHashWithoutSize {
        size_t operator()(const RHIRenderPassDescription& Desc) const;
    };
    struct RenderPassDescriptionEqualWithoutSize {
        size_t operator()(const RHIRenderPassDescription& A, const RHIRenderPassDescription& B) const;
    };

public:
    explicit RenderPassManager(VulkanDevice* InDevice);
    ~RenderPassManager();

    void Clear();

    WeakRef<VulkanRenderPass> Get(const RHIRenderPassDescription& Description);

public:
    VulkanDevice* Device;
    std::unordered_map<RHIRenderPassDescription, VkRenderPass, RenderPassDescriptionHashWithoutSize,
                       RenderPassDescriptionEqualWithoutSize>
        RenderPassStorage;
    std::unordered_map<RHIRenderPassDescription, Ref<VulkanRenderPass>> StorageMap;
};

}    // namespace VulkanRHI
