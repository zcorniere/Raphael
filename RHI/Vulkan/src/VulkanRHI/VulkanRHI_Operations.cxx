#include "VulkanRHI/Resources/VulkanBuffer.hxx"
#include "VulkanRHI/VulkanRHI.hxx"

#include "VulkanRHI/RenderPass/RenderPassManager.hxx"
#include "VulkanRHI/RenderPass/VulkanRenderPass.hxx"

#include "VulkanRHI/Resources/VulkanGraphicsPipeline.hxx"
#include "VulkanRHI/Resources/VulkanViewport.hxx"

#include "VulkanRHI/VulkanCommandsObjects.hxx"
#include "VulkanRHI/VulkanDevice.hxx"

#include "Engine/Core/Window.hxx"
#include "Engine/Misc/DataLocation.hxx"

namespace VulkanRHI
{

// RHI Operation default function
void VulkanDynamicRHI::BeginFrame()
{
    GetDevice()->CommandManager->PrepareForNewActiveCommandBuffer();
}

void VulkanDynamicRHI::Tick(float fDeltaTime)
{
    (void)fDeltaTime;
}

void VulkanDynamicRHI::EndFrame()
{
    ENQUEUE_RENDER_COMMAND(EndFrame)
    ([this] {
        if (DrawingViewport.IsValid()) {
            LOG(LogVulkanRHI, Error, "Viewport \"{}\"draw session was not ended properly !",
                DrawingViewport->GetName());
            DrawingViewport->RT_EndDrawViewport();
        }

        GetDevice()->GetCommandManager()->SubmitActiveCmdBuffer();
    });

    RHI::GetRHICommandQueue()->Execute();
}

void VulkanDynamicRHI::BeginRenderPass(const RHIRenderPassDescription& Renderpass,
                                       const RHIFramebufferDefinition& Framebuffer)
{
    ENQUEUE_RENDER_COMMAND(BeginRenderPass)
    ([this, Renderpass, Framebuffer] {
        CurrentRenderPass = RPassManager->GetRenderPass(Renderpass);
        check(CurrentRenderPass);

        WeakRef<VulkanFramebuffer> FramebufferRef = RPassManager->GetFrameBuffer(CurrentRenderPass.Raw(), Framebuffer);
        check(FramebufferRef);

        VulkanCmdBuffer* CmdBuffer = GetDevice()->GetCommandManager()->GetActiveCmdBuffer();
        CurrentRenderPass->Begin(CmdBuffer, FramebufferRef,
                                 {
                                     .offset =
                                         {
                                             .x = Framebuffer.Offset.x,
                                             .y = Framebuffer.Offset.y,
                                         },
                                     .extent =
                                         {
                                             .width = FramebufferRef->GetExtent().x,
                                             .height = FramebufferRef->GetExtent().y,
                                         },
                                 });
    });
}

void VulkanDynamicRHI::EndRenderPass()
{
    ENQUEUE_RENDER_COMMAND(EndRenderPass)
    ([this] {
        check(CurrentRenderPass.IsValid());
        VulkanCmdBuffer* CmdBuffer = GetDevice()->GetCommandManager()->GetActiveCmdBuffer();

        CurrentRenderPass->End(CmdBuffer);
    });
}

void VulkanDynamicRHI::Draw(Ref<RHIGraphicsPipeline>& Pipeline)
{
    ENQUEUE_RENDER_COMMAND(Draw)
    ([this, Pipeline] {
        VulkanCmdBuffer* CmdBuffer = GetDevice()->GetCommandManager()->GetActiveCmdBuffer();

        Ref<VulkanGraphicsPipeline> VulkanPipeline = Pipeline.As<VulkanGraphicsPipeline>();
        VulkanPipeline->Bind(CmdBuffer->GetHandle());

        VkViewport viewport{
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(DrawingViewport->GetSize().x),
            .height = static_cast<float>(DrawingViewport->GetSize().y),
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };
        VkRect2D scissor{
            .offset =
                {
                    .x = 0,
                    .y = 0,
                },
            .extent =
                {
                    .width = DrawingViewport->GetSize().x,
                    .height = DrawingViewport->GetSize().y,
                },
        };
        VulkanAPI::vkCmdSetViewport(CmdBuffer->GetHandle(), 0, 1, &viewport);
        VulkanAPI::vkCmdSetScissor(CmdBuffer->GetHandle(), 0, 1, &scissor);
        // DELETE ME
        VulkanAPI::vkCmdDraw(CmdBuffer->GetHandle(), 3, 1, 0, 0);
    });
}

void VulkanDynamicRHI::RT_SetDrawingViewport(WeakRef<VulkanViewport> Viewport)
{
    DrawingViewport = Viewport;
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
    WeakRef<VulkanRenderPass> VRenderPass = RPassManager->GetRenderPass(Config.RenderPass);
    check(VRenderPass);

    GraphicsPipelineDescription Desc;
    Desc.VertexShader = CreateShader(Config.VertexShader, false);
    Desc.PixelShader = CreateShader(Config.PixelShader, false);
    Desc.RenderPass = Ref(VRenderPass);
    Desc.Rasterizer.CullMode = ConvertToVulkanType(Config.Rasterizer.CullMode);
    Desc.Rasterizer.FrontFaceCulling = ConvertToVulkanType(Config.Rasterizer.FrontFaceCulling);
    Desc.Rasterizer.PolygonMode = ConvertToVulkanType(Config.Rasterizer.PolygonMode);
    check(Desc.Validate());

    return Ref<VulkanGraphicsPipeline>::Create(Device.get(), Desc);
}
}    // namespace VulkanRHI
