#pragma once

#include "Engine/Core/RHI/RHIDefinitions.hxx"
#include "VulkanRHI/RenderPass/VulkanRenderPass.hxx"

namespace VulkanRHI
{
class VulkanDevice;

/// This class manage the creation and deletion of Vulkan render passes
class RenderPassManager : public IDeviceChild
{
public:
    explicit RenderPassManager(VulkanDevice* InDevice);
    ~RenderPassManager();

    /// Delete all the renderpasses currently stored
    /// @note ClearFramebuffers() will be called
    void ClearRenderpass();
    /// Delete all framebuffers objects currently stored
    void ClearFramebuffers();

    WeakRef<VulkanRenderPass> GetRenderPass(const RHIRenderPassDescription& Description);
    WeakRef<VulkanFramebuffer> GetFrameBuffer(const VulkanRenderPass* const RenderPass,
                                              const RHIFramebufferDefinition& Definition);

public:
    std::unordered_map<RHIRenderPassDescription, VkRenderPass> RenderPassStorage;
    std::unordered_map<RHIRenderPassDescription, Ref<VulkanRenderPass>> RenderPassStorageMap;
    std::unordered_map<RHIFramebufferDefinition, Ref<VulkanFramebuffer>> FrameBufferStorageMap;
};

}    // namespace VulkanRHI
