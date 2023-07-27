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

VulkanGraphicsPipeline::VulkanGraphicsPipeline(VulkanDevice* InDevice, const GraphicsPipelineDescription& Description)
    : Device(InDevice), Desc(Description)
{
    Create(false);
}

VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
{
    if (VulkanPipeline) {
        VulkanAPI::vkDestroyPipeline(Device->GetHandle(), VulkanPipeline, VULKAN_CPU_ALLOCATOR);
    }
    if (PipelineLayout) {
        VulkanAPI::vkDestroyPipelineLayout(Device->GetHandle(), PipelineLayout, VULKAN_CPU_ALLOCATOR);
    }
}

void VulkanGraphicsPipeline::SetName(std::string_view Name)
{
    RObject::SetName(Name);
    if (VulkanPipeline) {
        VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_PIPELINE, VulkanPipeline, "{}", Name);
    }
    if (PipelineLayout) {
        VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_PIPELINE_LAYOUT, PipelineLayout, "{} [Pipeline Layout]", Name);
    }
}

bool VulkanGraphicsPipeline::Create(bool bForceRecompileShaders)
{
    Shaders[0] = RHI::CreateShader(Desc.VertexShader, bForceRecompileShaders);
    Shaders[1] = RHI::CreateShader(Desc.PixelShader, bForceRecompileShaders);

    CreatePipelineLayout();

    Array<VkPipelineShaderStageCreateInfo> ShaderStage;
    for (Ref<VulkanShader>& Shader: Shaders) {
        Ref<VulkanShader::ShaderHandle> Handle = Shader->GetHandle(Device);
        ShaderStage.Add(VkPipelineShaderStageCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = ConvertToVulkanType(Shader->GetShaderType()),
            .pName = "main",
        });
    }

    Array<VkVertexInputBindingDescription> InputBinding(Desc.VertexBindings.Size());
    for (unsigned i = 0; i < Desc.VertexBindings.Size(); i++) {
        Desc.VertexBindings[i].WriteInto(InputBinding[i]);
    }
    Array<VkVertexInputAttributeDescription> InputAttribute(Desc.VertexAttributes.Size());
    for (unsigned i = 0; i < Desc.VertexAttributes.Size(); i++) {
        Desc.VertexAttributes[i].WriteInto(InputAttribute[i]);
    }
    VkPipelineRasterizationStateCreateInfo RasterizerInfo;
    Desc.Rasterizer.WriteInto(RasterizerInfo);

    VkPipelineVertexInputStateCreateInfo VertexInputState{
        .vertexBindingDescriptionCount = InputBinding.Size(),
        .pVertexBindingDescriptions = InputBinding.Raw(),
        .vertexAttributeDescriptionCount = InputAttribute.Size(),
        .pVertexAttributeDescriptions = InputAttribute.Raw(),
    };
    VkPipelineInputAssemblyStateCreateInfo InputAssembly{
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    VkGraphicsPipelineCreateInfo PipelineCreateInfo{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stageCount = ShaderStage.Size(),
        .pStages = ShaderStage.Raw(),
        .pVertexInputState = &VertexInputState,
        .pInputAssemblyState = &InputAssembly,
        .layout = PipelineLayout,

    };

    VK_CHECK_RESULT(VulkanAPI::vkCreateGraphicsPipelines(Device->GetHandle(), nullptr, 1, &PipelineCreateInfo,
                                                         VULKAN_CPU_ALLOCATOR, &VulkanPipeline));
    return false;
}

static VkPushConstantRange GetConstantRangeFromShader(Array<VkPushConstantRange>& OutPushRanges,
                                                      Ref<VulkanShader>& InShader, const VkShaderStageFlags ShaderStage)
{
    VkPushConstantRange Range{
        .stageFlags = ShaderStage,
        .offset = 0,
        .size = 0,
    };
    for (const ShaderResource::PushConstantRange& PushConstant: InShader->GetReflectionData().PushConstants) {
        if (Range.offset < PushConstant.Offset) {
            Range.offset = PushConstant.Offset;
            Range.size = PushConstant.Size;
        }
    }
    if (Range.size != 0) {
        OutPushRanges.Add(Range);
    }
    return Range;
}

bool VulkanGraphicsPipeline::CreatePipelineLayout()
{
    Array<VkPushConstantRange> PushRanges;
    GetConstantRangeFromShader(PushRanges, Shaders[0], VK_SHADER_STAGE_VERTEX_BIT);
    GetConstantRangeFromShader(PushRanges, Shaders[1], VK_SHADER_STAGE_FRAGMENT_BIT);

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
    VK_CHECK_RESULT(
        VulkanAPI::vkCreatePipelineLayout(Device->GetHandle(), &CreateInfo, VULKAN_CPU_ALLOCATOR, &PipelineLayout));
    return true;
}

}    // namespace VulkanRHI
