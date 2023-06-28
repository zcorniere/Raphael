#pragma once

#include "Engine/Core/RHI/RHIDefinitions.hxx"
#include "VulkanRHI/RenderPass/VulkanRenderPass.hxx"

namespace VulkanRHI
{
class VulkanDevice;

class RenderPassManager
{
public:
    RenderPassManager(Ref<VulkanDevice>& InDevice);
    ~RenderPassManager();

    void Clear();

    WeakRef<VulkanRenderPass> Get(const RHIRenderPassDescription& Description);

public:
    Ref<VulkanDevice> Device;
    std::unordered_map<RHIRenderPassDescription, Ref<VulkanRenderPass>> StorageMap;
};

}    // namespace VulkanRHI
