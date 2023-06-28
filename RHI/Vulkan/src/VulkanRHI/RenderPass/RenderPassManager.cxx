#include "VulkanRHI/RenderPass/RenderPassManager.hxx"

#include "VulkanRHI/VulkanDevice.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogRenderPassManager, Info)

namespace VulkanRHI
{

RenderPassManager::RenderPassManager(Ref<VulkanDevice>& InDevice): Device(InDevice)
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
    StorageMap.clear();
}

WeakRef<VulkanRenderPass> RenderPassManager::Get(const RHIRenderPassDescription& Description)
{
    auto Iter = StorageMap.find(Description);
    if (Iter != StorageMap.end()) {
        return Iter->second;
    }

    Ref<VulkanRenderPass> Pass = Ref<VulkanRenderPass>::Create(Device, Description);
    StorageMap[Description] = Pass;
    return Pass;
}

}    // namespace VulkanRHI
