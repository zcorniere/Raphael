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

// void VulkanDynamicRHI::Draw(Ref<RHIGraphicsPipeline>& Pipeline)
// {
//     ENQUEUE_RENDER_COMMAND(Draw)
//     ([this, Pipeline] {
//         VulkanCmdBuffer* CmdBuffer = GetDevice()->GetCommandManager()->GetActiveCmdBuffer();

//         Ref<VulkanGraphicsPipeline> VulkanPipeline = Pipeline.As<VulkanGraphicsPipeline>();
//         VulkanPipeline->Bind(CmdBuffer->GetHandle());

//         VkViewport viewport{
//             .x = 0.0f,
//             .y = 0.0f,
//             .width = static_cast<float>(DrawingViewport->GetSize().x),
//             .height = static_cast<float>(DrawingViewport->GetSize().y),
//             .minDepth = 0.0f,
//             .maxDepth = 1.0f,
//         };
//         VkRect2D scissor{
//             .offset =
//                 {
//                     .x = 0,
//                     .y = 0,
//                 },
//             .extent =
//                 {
//                     .width = DrawingViewport->GetSize().x,
//                     .height = DrawingViewport->GetSize().y,
//                 },
//         };
//         VulkanAPI::vkCmdSetViewport(CmdBuffer->GetHandle(), 0, 1, &viewport);
//         VulkanAPI::vkCmdSetScissor(CmdBuffer->GetHandle(), 0, 1, &scissor);
//         // DELETE ME
//         VulkanAPI::vkCmdDraw(CmdBuffer->GetHandle(), 3, 1, 0, 0);
//     });
// }

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
    check(Desc.Validate());

    return Ref<VulkanGraphicsPipeline>::Create(Device.get(), Desc);
}
}    // namespace VulkanRHI
