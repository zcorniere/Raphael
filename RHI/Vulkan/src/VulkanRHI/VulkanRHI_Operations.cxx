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
        VulkanCmdBuffer* CmdBuffer = GetDevice()->GetCommandManager()->GetActiveCmdBuffer();

        CurrentRenderPass->End(CmdBuffer);
        CurrentRenderPass = nullptr;
    });
}

void VulkanDynamicRHI::RT_SetDrawingViewport(VulkanViewport* Viewport)
{
    // TODO: check if Viewport is inside Viewports (wtf no find in std::vector ?)
    // TODO: check if we are really on the renderthread once such thing exists
    DrawingViewport = Viewport;
}

//
//  -------------------- RHI Create resources --------------------
//

RHIViewportRef VulkanDynamicRHI::CreateViewport(void* InWindowHandle, glm::uvec2 InSize)
{
    return new VulkanViewport(GetDevice(), InWindowHandle, InSize);
}

RHITextureRef VulkanDynamicRHI::CreateTexture(const RHITextureCreateDesc InDesc)
{
    return new VulkanTexture(GetDevice(), InDesc);
}

RHIBufferRef VulkanDynamicRHI::CreateBuffer(const uint32 InSize, const EBufferUsageFlags InUsage, const uint32 InStride,
                                            ResourceArray* InitialData)
{
    return new VulkanBuffer(GetDevice(), InSize, InUsage, InStride, InitialData);
}

RHIShaderRef VulkanDynamicRHI::CreateShader(const std::filesystem::path Path, bool bForceCompile)
{
    std::filesystem::path RefPath = DataLocationFinder::GetShaderPath();
    VulkanShader* Shader = ShaderCompiler.Get(RefPath / Path, bForceCompile);
    check(Shader);
    return Shader;
}

RHIGraphicsPipelineRef VulkanRHI::VulkanDynamicRHI::CreateGraphicsPipeline(const RHIGraphicsPipelineInitializer& Config)
{
    GraphicsPipelineDescription Desc;
    Desc.VertexShader = Config.VertexShader;
    Desc.PixelShader = Config.PixelShader;
    Desc.Rasterizer.CullMode = ConvertToVulkanType(Config.Rasterizer.CullMode);
    Desc.Rasterizer.FrontFaceCulling = ConvertToVulkanType(Config.Rasterizer.FrontFaceCulling);
    Desc.Rasterizer.PolygonMode = ConvertToVulkanType(Config.Rasterizer.PolygonMode);

    return new VulkanGraphicsPipeline(Device, Desc);
}
}    // namespace VulkanRHI
