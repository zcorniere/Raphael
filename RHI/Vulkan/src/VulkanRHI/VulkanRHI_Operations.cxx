#include "VulkanRHI/VulkanRHI.hxx"

#include "VulkanRHI/VulkanCommandsObjects.hxx"
#include "VulkanRHI/VulkanDevice.hxx"

#include "Engine/Misc/DataLocation.hxx"

namespace VulkanRHI
{

// RHI Operation default function
void VulkanDynamicRHI::BeginFrame() {}

void VulkanDynamicRHI::EndFrame()
{
    RHI::Submit([] { GetVulkanDynamicRHI()->GetDevice()->CommandManager->SubmitActiveCmdBuffer(); });

    RHI::GetRHICommandQueue()->Execute();
}

void VulkanDynamicRHI::NextFrame() { GetDevice()->CommandManager->PrepareForNewActiveCommandBuffer(); }

void VulkanDynamicRHI::BeginDrawingViewport(Ref<RHIViewport> &RHIViewport)
{
    DrawingViewport = RHIViewport.As<VulkanViewport>();
}

void VulkanDynamicRHI::EndDrawingViewport(Ref<RHIViewport> &RHIViewport)
{
    Ref<VulkanViewport> Viewport = RHIViewport.As<VulkanViewport>();

    Ref<VulkanCmdBuffer> CmdBuffer = Device->GetCommandManager()->GetActiveCmdBuffer();
    check(!CmdBuffer->HasEnded() && !CmdBuffer->IsInsideRenderPass());

    Viewport->Present(CmdBuffer, Device->GraphicsQueue, Device->PresentQueue);
}

//
//  -------------------- RHI Create resources --------------------
//

Ref<RHIViewport> VulkanDynamicRHI::CreateViewport(void *InWindowHandle, glm::uvec2 InSize)
{
    Viewports.push_back(Ref<VulkanViewport>::Create(GetDevice(), InWindowHandle, InSize));
    return Viewports.back();
}

Ref<RHITexture> VulkanDynamicRHI::CreateTexture(const RHITextureCreateDesc InDesc)
{
    return Ref<VulkanTexture>::Create(GetDevice(), InDesc);
}

Ref<RHIShader> VulkanDynamicRHI::CreateShader(const std::filesystem::path Path, bool bForceCompile)
{
    std::filesystem::path RefPath = DataLocationFinder::GetShaderPath();
    Ref<VulkanShader> Shader = ShaderCompiler.Get(RefPath / Path, bForceCompile);
    check(Shader);
    return Shader;
}
}    // namespace VulkanRHI
