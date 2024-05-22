#include "VulkanRHI/Resources/VulkanBuffer.hxx"
#include "VulkanRHI/VulkanCommandContext.hxx"
#include "VulkanRHI/VulkanRHI.hxx"

#include "VulkanRHI/Resources/VulkanGraphicsPipeline.hxx"
#include "VulkanRHI/Resources/VulkanViewport.hxx"

#include "VulkanRHI/VulkanCommandsObjects.hxx"
#include "VulkanRHI/VulkanDevice.hxx"

#include "Engine/Core/Window.hxx"
#include "Engine/Misc/DataLocation.hxx"

namespace VulkanRHI
{

void VulkanDynamicRHI::RHISubmitCommandLists(RHICommandList* const CommandLists, std::uint32_t NumCommandLists)
{
    (void)CommandLists;
    (void)NumCommandLists;
    GetDevice()->GetCommandManager()->SubmitActiveCmdBuffer();
}

RHIContext* VulkanDynamicRHI::RHIGetCommandContext()
{
    return new VulkanCommandContext(Device.get(), Device->GraphicsQueue.get(), Device->PresentQueue);
}

void VulkanDynamicRHI::RHIReleaseCommandContext(RHIContext* Context)
{
    delete Context;
}

void VulkanDynamicRHI::WaitUntilIdle()
{
    Device->WaitUntilIdle();
}

//
//  -------------------- RHI Create resources --------------------
//

Ref<RHIViewport> VulkanDynamicRHI::CreateViewport(Ref<Window> InWindowHandle, glm::uvec2 InSize)
{
    return Ref<VulkanViewport>::Create(GetDevice(), std::move(InWindowHandle), std::move(InSize));
}

Ref<RHITexture> VulkanDynamicRHI::CreateTexture(const RHITextureSpecification& InDesc)
{
    return Ref<VulkanTexture>::CreateNamed(InDesc.Name, GetDevice(), InDesc);
}

Ref<RHIBuffer> VulkanDynamicRHI::CreateBuffer(const RHIBufferDesc& InDesc)
{
    Ref<VulkanBuffer> Buffer = Ref<VulkanBuffer>::Create(GetDevice(), InDesc);
    Buffer->SetName(InDesc.DebugName.empty() ? "Unknown buffer" : InDesc.DebugName);
    return Buffer;
}

Ref<RHIShader> VulkanDynamicRHI::CreateShader(const std::filesystem::path Path, bool bForceCompile)
{
    std::filesystem::path RefPath = DataLocationFinder::GetShaderPath();
    Ref<VulkanShader> Shader = ShaderCompiler->Get(RefPath / Path, bForceCompile);
    check(Shader);
    return Shader;
}

Ref<RHIGraphicsPipeline>
VulkanRHI::VulkanDynamicRHI::CreateGraphicsPipeline(const RHIGraphicsPipelineSpecification& Config)
{
    GraphicsPipelineDescription Desc;
    Desc.VertexShader = CreateShader(Config.VertexShader, false);
    Desc.PixelShader = CreateShader(Config.PixelShader, false);
    Desc.Rasterizer.CullMode = ConvertToVulkanType(Config.Rasterizer.CullMode);
    Desc.Rasterizer.FrontFaceCulling = ConvertToVulkanType(Config.Rasterizer.FrontFaceCulling);
    Desc.Rasterizer.PolygonMode = ConvertToVulkanType(Config.Rasterizer.PolygonMode);
    Desc.AttachmentFormats = Config.AttachmentFormats;

    if (Desc.VertexAttributes.IsEmpty()) {
        Desc.VertexAttributes = Desc.VertexShader->GetReflectionData().GetInputVertexAttributes();
    }
    if (Desc.VertexBindings.IsEmpty()) {
        Desc.VertexBindings = Desc.VertexShader->GetReflectionData().GetInputVertexBindings();
    }

    check(Desc.Validate());

    return Ref<VulkanGraphicsPipeline>::Create(Device.get(), Desc);
}
}    // namespace VulkanRHI
