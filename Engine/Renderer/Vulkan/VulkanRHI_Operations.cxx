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

template <>
Ref<RHIResource> VulkanDynamicRHI::Create<RHIResourceType::Viewport>(void *InWindowHandle, glm::uvec2 InSize)
{
    return Ref<VulkanViewport>::Create(GetVulkanDynamicRHI()->GetDevice(), InWindowHandle, InSize);
}

template <>
Ref<RHIResource> VulkanDynamicRHI::Create<RHIResourceType::Texture>(const RHITextureCreateDesc &InDesc)
{
    return Ref<VulkanTexture>::Create(GetVulkanDynamicRHI()->GetDevice(), InDesc);
}

template <>
Ref<RHIResource> VulkanDynamicRHI::Create<RHIResourceType::Shader>(const std::filesystem::path &Path,
                                                                   bool bForceCompile)
{
    return GetVulkanDynamicRHI()->ShaderCompiler.Get(Path, bForceCompile);
}

}    // namespace VulkanRHI
