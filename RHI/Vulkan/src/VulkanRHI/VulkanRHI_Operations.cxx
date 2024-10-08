#include "Engine/Core/Engine.hxx"
#include "VulkanRHI/Resources/VulkanBuffer.hxx"
#include "VulkanRHI/VulkanCommandContext.hxx"
#include "VulkanRHI/VulkanRHI.hxx"

#include "VulkanRHI/Resources/VulkanGraphicsPipeline.hxx"
#include "VulkanRHI/Resources/VulkanViewport.hxx"

#include "VulkanRHI/VulkanCommandContext.hxx"
#include "VulkanRHI/VulkanCommandsObjects.hxx"
#include "VulkanRHI/VulkanDevice.hxx"

#include "Engine/Core/RHI/RHICommandList.hxx"
#include "Engine/Core/Window.hxx"
#include "Engine/Misc/DataLocation.hxx"

namespace VulkanRHI
{

void VulkanDynamicRHI::RHISubmitCommandLists(RHICommandList* const CommandLists, std::uint32_t NumCommandLists)
{
    for (std::uint32_t i = 0; i < NumCommandLists; ++i) {
        VulkanCommandContext* Context = static_cast<VulkanCommandContext*>(CommandLists[i].GetContext());
        Context->GetCommandManager()->SubmitActiveCmdBuffer();
    }
}

RHIContext* VulkanDynamicRHI::RHIGetCommandContext()
{
    VulkanCommandContext* Context = nullptr;
    if (AvailableCommandContexts.IsEmpty()) {
        Context = new VulkanCommandContext(Device.get(), Device->GraphicsQueue.get(), Device->PresentQueue);
    } else {
        Context = AvailableCommandContexts.Pop();
        Context->GetCommandManager()->RefreshFenceStatus();
    }
    CommandContexts.Add(Context);

    return Context;
}

void VulkanDynamicRHI::RHIReleaseCommandContext(RHIContext* Context)
{
    VulkanCommandContext* VulkanContext = static_cast<VulkanCommandContext*>(Context);
    CommandContexts.Remove(VulkanContext);
    AvailableCommandContexts.Add(VulkanContext);
}

void VulkanDynamicRHI::WaitUntilIdle()
{
    Device->WaitUntilIdle();
    for (VulkanCommandContext* Context: CommandContexts) {
        Context->GetCommandManager()->RefreshFenceStatus();
    }
}

//
//  -------------------- RHI Create resources --------------------
//

Ref<RHIViewport> VulkanDynamicRHI::CreateViewport(Ref<Window> InWindowHandle, UVector2 InSize)
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
    if (!InDesc.DebugName.empty()) {
        Buffer->SetName(InDesc.DebugName);
    }
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
    std::future<Ref<RHIShader>> VertexShader = RHI::CreateShaderAsync(Config.VertexShader, false);
    std::future<Ref<RHIShader>> PixelShader = RHI::CreateShaderAsync(Config.PixelShader, false);

    GraphicsPipelineDescription Desc;
    Desc.Rasterizer.CullMode = ConvertToVulkanType(Config.Rasterizer.CullMode);
    Desc.Rasterizer.FrontFaceCulling = ConvertToVulkanType(Config.Rasterizer.FrontFaceCulling);
    Desc.Rasterizer.PolygonMode = ConvertToVulkanType(Config.Rasterizer.PolygonMode);
    Desc.AttachmentFormats = Config.AttachmentFormats;
    Desc.VertexShader = VertexShader.get();
    Desc.PixelShader = PixelShader.get();

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
