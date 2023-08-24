#include "VulkanRHI/VulkanRHI.hxx"

#include "VulkanRHI/RenderPass/RenderPassManager.hxx"
#include "VulkanRHI/RenderPass/VulkanRenderPass.hxx"
#include "VulkanRHI/VulkanCommandsObjects.hxx"
#include "VulkanRHI/VulkanDevice.hxx"

#include "Engine/Misc/DataLocation.hxx"
#include "VulkanRHI.hxx"

namespace VulkanRHI
{

// RHI Operation default function
void VulkanDynamicRHI::BeginFrame()
{
    GetDevice()->CommandManager->PrepareForNewActiveCommandBuffer();
}

void VulkanDynamicRHI::EndFrame()
{
    RHI::Submit([this] {
        GetDevice()->GetCommandManager()->SubmitActiveCmdBuffer();

        if (DrawingViewport.IsValid()) {
            LOG(LogVulkanRHI, Error, "Viewport \"{}\"draw session was not ended properly !",
                DrawingViewport->GetName());
            DrawingViewport->RT_EndDrawViewport();
        }
    });

    RHI::GetRHICommandQueue()->Execute();
}

void VulkanDynamicRHI::NextFrame()
{
}

void VulkanDynamicRHI::BeginRenderPass(const RHIRenderPassDescription& Description)
{
    RHI::Submit([this, Description] {
        CurrentRenderPass = RPassManager->Get(Description);
        check(CurrentRenderPass);

        CurrentRenderPass->SetName(Description.Name);

        VulkanCmdBuffer* CmdBuffer = GetDevice()->GetCommandManager()->GetActiveCmdBuffer();
        CurrentRenderPass->Begin(CmdBuffer, {
                                                .offset =
                                                    {
                                                        .x = Description.Offset.x,
                                                        .y = Description.Offset.y,
                                                    },
                                                .extent =
                                                    {
                                                        .width = Description.Size.x,
                                                        .height = Description.Size.y,
                                                    },
                                            });
    });
}

void VulkanDynamicRHI::EndRenderPass()
{
    RHI::Submit([this] {
        check(CurrentRenderPass.IsValid());
        VulkanCmdBuffer* CmdBuffer = GetDevice()->GetCommandManager()->GetActiveCmdBuffer();

        CurrentRenderPass->End(CmdBuffer);
    });
}

void VulkanDynamicRHI::Draw(Ref<RHIGraphicsPipeline>& Pipeline)
{
    RHI::Submit([this, Pipeline] {
        VulkanCmdBuffer* CmdBuffer = GetDevice()->GetCommandManager()->GetActiveCmdBuffer();

        Ref<VulkanGraphicsPipeline> VulkanPipeline = Pipeline.As<VulkanGraphicsPipeline>();
        VulkanPipeline->Bind(CmdBuffer->GetHandle());

        // DELETE ME
        VulkanAPI::vkCmdDraw(CmdBuffer->GetHandle(), 3, 1, 0, 0);
    });
}

void VulkanDynamicRHI::RT_SetDrawingViewport(WeakRef<VulkanViewport> Viewport)
{
    // TODO: check if Viewport is inside Viewports (wtf no find in std::vector ?)
    // TODO: check if we are really on the renderthread once such thing exists
    DrawingViewport = Viewport;
}

//
//  -------------------- RHI Create resources --------------------
//

Ref<RHIViewport> VulkanDynamicRHI::CreateViewport(void* InWindowHandle, glm::uvec2 InSize)
{
    return Ref<VulkanViewport>::Create(GetDevice(), InWindowHandle, InSize);
}

Ref<RHITexture> VulkanDynamicRHI::CreateTexture(const RHITextureSpecification& InDesc)
{
    return Ref<VulkanTexture>::Create(GetDevice(), InDesc);
}

Ref<RHIBuffer> VulkanDynamicRHI::CreateBuffer(const uint32 InSize, const EBufferUsageFlags InUsage,
                                              const uint32 InStride, Ref<ResourceArray>& InitialData)
{
    return Ref<VulkanBuffer>::Create(GetDevice(), InSize, InUsage, InStride, InitialData);
}

Ref<RHIShader> VulkanDynamicRHI::CreateShader(const std::filesystem::path Path, bool bForceCompile)
{
    std::filesystem::path RefPath = DataLocationFinder::GetShaderPath();
    Ref<VulkanShader> Shader = ShaderCompiler.Get(RefPath / Path, bForceCompile);
    check(Shader);
    return Shader;
}

Ref<RHIGraphicsPipeline>
VulkanRHI::VulkanDynamicRHI::CreateGraphicsPipeline(const RHIGraphicsPipelineSpecification& Config)
{
    WeakRef<VulkanRenderPass> VRenderPass = RPassManager->Get(Config.RenderPass);
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
