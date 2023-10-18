#include "VulkanRHI/RenderPass/RenderPassManager.hxx"

#include "VulkanRHI/VulkanDevice.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogRenderPassManager, Info)

namespace VulkanRHI
{

RenderPassManager::RenderPassManager(VulkanDevice* InDevice): Device(InDevice)
{
}

RenderPassManager::~RenderPassManager()
{
    ClearRenderpass();
}

void RenderPassManager::ClearRenderpass()
{
    ClearFramebuffers();
    LOG(LogRenderPassManager, Info, "Clearing {} render pass{}", RenderPassStorageMap.size(),
        (RenderPassStorageMap.size() > 1) ? ("es") : (""));
    LOG(LogRenderPassManager, Info, "- {} vulkan render pass{}", RenderPassStorage.size(),
        (RenderPassStorage.size() > 1) ? ("es") : (""));

    RenderPassStorageMap.clear();

    for (auto& [_, VulkanPass]: RenderPassStorage) {
        VulkanAPI::vkDestroyRenderPass(Device->GetHandle(), VulkanPass, VULKAN_CPU_ALLOCATOR);
    }
    RenderPassStorage.clear();
}

void RenderPassManager::ClearFramebuffers()
{
    LOG(LogRenderPassManager, Info, "Clearing {} framebuffers pass{}", FrameBufferStorageMap.size(),
        (FrameBufferStorageMap.size() > 1) ? ("es") : (""));

    FrameBufferStorageMap.clear();
}

WeakRef<VulkanRenderPass> RenderPassManager::GetRenderPass(const RHIRenderPassDescription& Description)
{
    // Find an already created RenderPass
    auto RenderPassIter = RenderPassStorageMap.find(Description);
    if (RenderPassIter != RenderPassStorageMap.end()) {
        return RenderPassIter->second;
    }

    // None was found, so we need to create a new one, but check if a compatible Vulkan pass is available
    VkRenderPass RenderPass = VK_NULL_HANDLE;
    auto VulkanRenderPassIter = RenderPassStorage.find(Description);
    if (VulkanRenderPassIter != RenderPassStorage.end()) {
        RenderPass = VulkanRenderPassIter->second;
    }

    // Create the render pass
    Ref<VulkanRenderPass> Pass = Ref<VulkanRenderPass>::Create(Device, Description, RenderPass);

    // If no VkRenderPass was found earlier, store the new one
    if (!RenderPass) {
        RenderPassStorage[Description] = Pass->GetRenderPass();
    }
    RenderPassStorageMap[Description] = Pass;
    return Pass;
}

WeakRef<VulkanFramebuffer> RenderPassManager::GetFrameBuffer(const VulkanRenderPass* const RenderPass,
                                                             const RHIFramebufferDefinition& Definition)
{
    // Find an already created Framebuffer
    auto FramebufferIter = FrameBufferStorageMap.find(Definition);
    if (FramebufferIter != FrameBufferStorageMap.end()) {
        return FramebufferIter->second;
    }

    // Create the render pass
    Ref<VulkanFramebuffer> Framebuffer = Ref<VulkanFramebuffer>::Create(Device, RenderPass, Definition);

    // If no VkFramebuffer was found earlier, store the new one
    FrameBufferStorageMap[Definition] = Framebuffer;
    return Framebuffer;
}

}    // namespace VulkanRHI
