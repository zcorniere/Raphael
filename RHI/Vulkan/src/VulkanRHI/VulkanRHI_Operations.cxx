#include "VulkanRHI/Resources/VulkanBuffer.hxx"
#include "VulkanRHI/Resources/VulkanMaterial.hxx"
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

void FVulkanDynamicRHI::RHISubmitCommandLists(FFRHICommandList* const CommandLists, std::uint32_t NumCommandLists)
{
    for (std::uint32_t i = 0; i < NumCommandLists; ++i) {
        FVulkanCommandContext* Context = static_cast<FVulkanCommandContext*>(CommandLists[i].GetContext());
        Context->GetCommandManager()->SubmitActiveCmdBuffer();
    }
}

FRHIContext* FVulkanDynamicRHI::RHIGetCommandContext()
{
    FVulkanCommandContext* Context = nullptr;
    if (AvailableCommandContexts.IsEmpty()) {
        Context = new FVulkanCommandContext(Device.get(), Device->GraphicsQueue.get(), Device->PresentQueue);
    } else {
        Context = AvailableCommandContexts.Pop();
        Context->GetCommandManager()->RefreshFenceStatus();
    }
    CommandContexts.Add(Context);

    return Context;
}

void FVulkanDynamicRHI::RHIReleaseCommandContext(FRHIContext* Context)
{
    FVulkanCommandContext* VulkanContext = static_cast<FVulkanCommandContext*>(Context);
    CommandContexts.Remove(VulkanContext);
    AvailableCommandContexts.Add(VulkanContext);
}

void FVulkanDynamicRHI::WaitUntilIdle()
{
    Device->WaitUntilIdle();
    for (FVulkanCommandContext* Context: CommandContexts) {
        Context->GetCommandManager()->RefreshFenceStatus();
    }
}

//
//  -------------------- RHI Create resources --------------------
//

Ref<RRHIViewport> FVulkanDynamicRHI::CreateViewport(Ref<RWindow> InWindowHandle, UVector2 InSize,
                                                    bool bCreateDepthBuffer)
{
    return Ref<RVulkanViewport>::Create(GetDevice(), std::move(InWindowHandle), std::move(InSize), bCreateDepthBuffer);
}

Ref<RRHITexture> FVulkanDynamicRHI::CreateTexture(const FRHITextureSpecification& InDesc)
{
    return Ref<RVulkanTexture>::CreateNamed(InDesc.Name, GetDevice(), InDesc);
}

Ref<RRHIBuffer> FVulkanDynamicRHI::CreateBuffer(const FRHIBufferDesc& InDesc)
{
    Ref<RVulkanBuffer> Buffer = Ref<RVulkanBuffer>::Create(GetDevice(), InDesc);
    if (!InDesc.DebugName.empty()) {
        Buffer->SetName(InDesc.DebugName);
    }
    return Buffer;
}

Ref<RRHIShader> FVulkanDynamicRHI::CreateShader(const std::filesystem::path Path, bool bForceCompile)
{
    std::filesystem::path RefPath = DataLocationFinder::GetShaderPath();
    Ref<RVulkanShader> Shader = ShaderCompiler->Get(RefPath / Path, bForceCompile);
    return Shader;
}

Ref<RRHIGraphicsPipeline>
VulkanRHI::FVulkanDynamicRHI::CreateGraphicsPipeline(const FRHIGraphicsPipelineSpecification& Config)
{
    std::future<Ref<RRHIShader>> VertexShader = RHI::CreateShaderAsync(Config.VertexShader, false);
    std::future<Ref<RRHIShader>> FragmentShader = RHI::CreateShaderAsync(Config.FragmentShader, false);

    FGraphicsPipelineDescription Desc;
    Desc.Rasterizer.CullMode = ConvertToVulkanType(Config.Rasterizer.CullMode);
    Desc.Rasterizer.FrontFaceCulling = ConvertToVulkanType(Config.Rasterizer.FrontFaceCulling);
    Desc.Rasterizer.PolygonMode = ConvertToVulkanType(Config.Rasterizer.PolygonMode);
    Desc.AttachmentFormats = Config.AttachmentFormats;

    unsigned NextLocation = 0;
    for (unsigned BufferLayoutIndex = 0; BufferLayoutIndex < Config.VertexBufferLayouts.Size(); BufferLayoutIndex++) {
        const FRHIGraphicsPipelineSpecification::FVertexBufferLayout& Layout =
            Config.VertexBufferLayouts[BufferLayoutIndex];

        // Compute the stride and offsets of the provided layout and
        // Set the vertex attributes with the components in the layout
        unsigned Stride = 0;
        for (unsigned ElementIndex = 0; ElementIndex < Layout.Parameter.Size(); ElementIndex++) {
            const FRHIGraphicsPipelineSpecification::FVertexBufferLayout::FElement& Element =
                Layout.Parameter[ElementIndex];

            FGraphicsPipelineDescription::FVertexAttribute& Attribute = Desc.VertexAttributes.Emplace();
            Attribute.Binding = BufferLayoutIndex;
            Attribute.Format = Element.Type;
            Attribute.Location = NextLocation++;
            Attribute.Offset = Stride;

            Stride += GetSizeOfElementType(Layout.Parameter[ElementIndex].Type);
        }

        // Set the vertex binding with the new info
        FGraphicsPipelineDescription::FVertexBinding& Binding = Desc.VertexBindings.Emplace();
        Binding.InputRate = ConvertToVulkanType(Layout.InputMode);
        Binding.Stride = Stride;
        Binding.Binding = BufferLayoutIndex;
    }

    Desc.VertexShader = VertexShader.get();
    Desc.FragmentShader = FragmentShader.get();
    if (!Desc.Validate()) {
        return nullptr;
    }
    return Ref<RVulkanGraphicsPipeline>::Create(Device.get(), Desc);
}

Ref<RRHIMaterial> FVulkanDynamicRHI::CreateMaterial(const WeakRef<RRHIGraphicsPipeline>& Pipeline)
{
    if (!Pipeline->IsValid()) {
        return nullptr;
    }
    Ref<RVulkanGraphicsPipeline> PipelineRef = Pipeline.Pin();

    Ref<RVulkanMaterial> Material = Ref<RVulkanMaterial>::Create(Device.get(), PipelineRef);
    return Material;
}

}    // namespace VulkanRHI
