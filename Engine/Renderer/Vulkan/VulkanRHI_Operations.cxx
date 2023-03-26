#include "Engine/Renderer/Vulkan/VulkanRHI.hxx"

#include "Engine/Renderer/Vulkan/VulkanCommandsObjects.hxx"
#include "Engine/Renderer/Vulkan/VulkanDevice.hxx"

namespace VulkanRHI
{

// RHI Operation default function
void VulkanDynamicRHI::BeginFrame()
{
}

void VulkanDynamicRHI::EndFrame()
{
    RHI::Submit([] { GetVulkanDynamicRHI()->GetDevice()->CommandManager->SubmitActiveCmdBuffer(); });
    GetVulkanDynamicRHI()->GetRHICommandQueue()->Execute();
}

void VulkanDynamicRHI::NextFrame()
{
    GetVulkanDynamicRHI()->GetDevice()->CommandManager->PrepareForNewActiveCommandBuffer();
}

}    // namespace VulkanRHI
