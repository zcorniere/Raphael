#include "VulkanRHI/RenderPass/RenderPassManager.hxx"

#include "VulkanRHI/VulkanDevice.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogRenderPassManager, Info)

namespace VulkanRHI
{

size_t RenderPassManager::RenderPassDescriptionHashWithoutSize::operator()(const RHIRenderPassDescription& Desc) const
{
    size_t Result = 0;
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

size_t RenderPassManager::RenderPassDescriptionEqualWithoutSize::operator()(const RHIRenderPassDescription& A,
                                                                            const RHIRenderPassDescription& B) const
{
    return A.ColorTarget == B.ColorTarget && A.ResolveTarget == B.ResolveTarget && A.DepthTarget == B.DepthTarget;
}

RenderPassManager::RenderPassManager(VulkanDevice* InDevice): Device(InDevice)
{
}

RenderPassManager::~RenderPassManager()
{
    Clear();
}

void RenderPassManager::Clear()
{
    LOG(LogRenderPassManager, Info, "Clearing {} render pass{}", StorageMap.size(),
        (StorageMap.size() > 1) ? ("es") : (""));
    LOG(LogRenderPassManager, Info, "- {} vulkan render pass{}", RenderPassStorage.size(),
        (RenderPassStorage.size() > 1) ? ("es") : (""));

    StorageMap.clear();

    for (auto& [_, VulkanPass]: RenderPassStorage) {
        VulkanAPI::vkDestroyRenderPass(Device->GetHandle(), VulkanPass, VULKAN_CPU_ALLOCATOR);
    }
    RenderPassStorage.clear();
}

WeakRef<VulkanRenderPass> RenderPassManager::Get(const RHIRenderPassDescription& Description)
{
    // Find an already created RenderPass
    auto RenderPassIter = StorageMap.find(Description);
    if (RenderPassIter != StorageMap.end()) {
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
    StorageMap[Description] = Pass;
    return Pass;
}

}    // namespace VulkanRHI
