#include "VulkanRHI/VulkanRHI.hxx"

#include "VulkanRHI/VulkanCommandsObjects.hxx"
#include "VulkanRHI/VulkanDevice.hxx"

#include "Engine/Misc/DataLocation.hxx"
#include "VulkanRHI.hxx"

namespace VulkanRHI
{

// RHI Operation default function
void VulkanDynamicRHI::BeginFrame()
{
}

void VulkanDynamicRHI::EndFrame()
{
    RHI::Submit([] { GetVulkanDynamicRHI()->GetDevice()->CommandManager->SubmitActiveCmdBuffer(); });

    RHI::GetRHICommandQueue()->Execute();
}

void VulkanDynamicRHI::NextFrame()
{
    GetDevice()->CommandManager->PrepareForNewActiveCommandBuffer();
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
    return Viewports.Add(Ref<VulkanViewport>::Create(GetDevice(), InWindowHandle, InSize));
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

Ref<RHIGraphicsPipeline>
VulkanRHI::VulkanDynamicRHI::CreateGraphicsPipeline(const RHIGraphicsPipelineInitializer& Config)
{
    GraphicsPipelineDescription Desc;
    Desc.VertexShader = Config.VertexShader;
    Desc.PixelShader = Config.PixelShader;
    Desc.Rasterizer.CullMode = ConvertToVulkanType(Config.Rasterizer.CullMode);
    Desc.Rasterizer.FrontFaceCulling = ConvertToVulkanType(Config.Rasterizer.FrontFaceCulling);
    Desc.Rasterizer.PolygonMode = ConvertToVulkanType(Config.Rasterizer.PolygonMode);

    return Ref<VulkanGraphicsPipeline>::Create(Device, Desc);
}
}    // namespace VulkanRHI
