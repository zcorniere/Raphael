#include "VulkanRHI/Resources/VulkanGraphicsPipeline.hxx"

#include "VulkanGraphicsPipeline.hxx"
#include "VulkanRHI/Resources/VulkanShader.hxx"
#include "VulkanRHI/VulkanDevice.hxx"

namespace VulkanRHI
{

void GraphicsPipelineDescription::VertexBinding::WriteInto(VkVertexInputBindingDescription& OutState) const
{
    OutState.binding = Binding;
    OutState.inputRate = (VkVertexInputRate)InputRate;
    OutState.stride = Stride;
}

void GraphicsPipelineDescription::VertexAttribute::WriteInto(VkVertexInputAttributeDescription& OutState) const
{
    OutState.binding = Binding;
    OutState.format = VertexElementToFormat(Format);
    OutState.location = Location;
    OutState.offset = Offset;
}

void GraphicsPipelineDescription::Rasterizer::WriteInto(VkPipelineRasterizationStateCreateInfo& OutState) const
{
    OutState.polygonMode = PolygonMode;
    OutState.cullMode = CullMode;
    OutState.frontFace = FrontFaceCulling;
    OutState.depthClampEnable = VK_FALSE;
    OutState.depthBiasEnable = VK_FALSE;
    OutState.rasterizerDiscardEnable = VK_FALSE;
    OutState.depthBiasSlopeFactor = 0.0f;
    OutState.depthBiasConstantFactor = 0.0f;
    OutState.lineWidth = 1.0f;
}

void GraphicsPipelineDescription::RenderTargets::AttachmentRef::WriteInto(VkAttachmentReference& OutState) const
{
    OutState.attachment = Attachment;
    OutState.layout = OutState.layout;
}

void VulkanRHI::GraphicsPipelineDescription::RenderTargets::AttachmentDesc::WriteInto(
    VkAttachmentDescription& OutState) const
{
    OutState.format = ImageFormatToFormat(Format);
    OutState.flags = Flags;
    OutState.samples = VK_SAMPLE_COUNT_1_BIT;
    OutState.loadOp = LoadOp;
    OutState.storeOp = StoreOp;
    OutState.initialLayout = InitialLayout;
    OutState.finalLayout = FinalLayout;
}

VulkanGraphicsPipeline::VulkanGraphicsPipeline(Ref<VulkanDevice>& InDevice,
                                               const GraphicsPipelineDescription& Description)
    : Device(InDevice), Desc(Description)
{
}

VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
{
    if (VulkanPipeline) {
        VulkanAPI::vkDestroyPipeline(Device->GetHandle(), VulkanPipeline, nullptr);
    }
    if (PipelineLayout) {
        VulkanAPI::vkDestroyPipelineLayout(Device->GetHandle(), PipelineLayout, nullptr);
    }
}

bool VulkanGraphicsPipeline::Create(bool bForceRecompileShaders)
{
    Shaders[0] = RHI::CreateShader(Desc.VertexShader, bForceRecompileShaders);
    Shaders[1] = RHI::CreateShader(Desc.PixelShader, bForceRecompileShaders);

    CreatePipelineLayout();

    VkGraphicsPipelineCreateInfo PipelineCreateInfo{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .layout = PipelineLayout,
    };

    VK_CHECK_RESULT(VulkanAPI::vkCreateGraphicsPipelines(Device->GetHandle(), nullptr, 1, &PipelineCreateInfo, nullptr,
                                                         &VulkanPipeline));
    return false;
}

static VkPushConstantRange GetConstantRangeFromShader(Ref<VulkanShader>& Shader, const VkShaderStageFlags ShaderStage)
{
    VkPushConstantRange Range{
        .stageFlags = ShaderStage,
        .offset = 0,
        .size = 0,
    };
    for (const ShaderResource::PushConstantRange& PushConstant: Shader->GetReflectionData().PushConstants) {
        if (Range.offset < PushConstant.Offset) {
            Range.offset = PushConstant.Offset;
            Range.size = PushConstant.Size;
        }
    }
    return Range;
}

bool VulkanGraphicsPipeline::CreatePipelineLayout()
{
    Array<VkPushConstantRange> PushRanges{
        GetConstantRangeFromShader(Shaders[0], VK_SHADER_STAGE_VERTEX_BIT),
        GetConstantRangeFromShader(Shaders[1], VK_SHADER_STAGE_FRAGMENT_BIT),
    };

    // TODO: Shader descriptor set reflection
    Array<VkDescriptorSetLayout> SetLayout;

    VkPipelineLayoutCreateInfo CreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .setLayoutCount = SetLayout.Size(),
        .pSetLayouts = SetLayout.Raw(),
        .pushConstantRangeCount = PushRanges.Size(),
        .pPushConstantRanges = PushRanges.Raw(),
    };
    VK_CHECK_RESULT(VulkanAPI::vkCreatePipelineLayout(Device->GetHandle(), &CreateInfo, nullptr, &PipelineLayout));
    return true;
}

}    // namespace VulkanRHI
