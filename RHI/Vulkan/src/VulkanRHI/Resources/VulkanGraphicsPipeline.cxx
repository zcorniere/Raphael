#include "VulkanRHI/Resources/VulkanGraphicsPipeline.hxx"

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

bool GraphicsPipelineDescription::Validate() const
{
    // Vertex shader is required
    if (!VertexShader.IsValid())
        return false;
    // Pixel and vertex shader must be different
    if (VertexShader == PixelShader)
        return false;
    // If there is a pixel shader, it must be valid
    if (PixelShader != nullptr && !PixelShader.IsValid())
        return false;

    return true;
}

VulkanGraphicsPipeline::VulkanGraphicsPipeline(VulkanDevice* InDevice, const GraphicsPipelineDescription& Description)
    : IDeviceChild(InDevice), Desc(Description)
{
    Create();
}

VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
{
    Device->WaitUntilIdle();
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
        VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_PIPELINE, VulkanPipeline, "{:s}", Name);
    }
    if (PipelineLayout) {
        VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_PIPELINE_LAYOUT, PipelineLayout, "{:s}.Layout", Name);
    }
}

static void FillShaderStageInfo(VulkanDevice* InDevice, Ref<VulkanShader>& InShader,
                                Array<VkPipelineShaderStageCreateInfo>& OutShaderStage)
{
    Ref<VulkanShader::ShaderHandle> Handle = InShader->GetHandle(InDevice);
    OutShaderStage.Add(VkPipelineShaderStageCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = ConvertToVulkanType(InShader->GetShaderType()),
        .module = Handle->Handle,
        .pName = "main",
    });
}

bool VulkanGraphicsPipeline::Create()
{
    CreatePipelineLayout();

    Array<VkPipelineShaderStageCreateInfo> ShaderStage;
    FillShaderStageInfo(Device, Desc.VertexShader, ShaderStage);
    FillShaderStageInfo(Device, Desc.PixelShader, ShaderStage);

    Array<VkVertexInputBindingDescription> InputBinding(Desc.VertexBindings.Size());
    for (unsigned i = 0; i < Desc.VertexBindings.Size(); i++) {
        Desc.VertexBindings[i].WriteInto(InputBinding[i]);
    }
    Array<VkVertexInputAttributeDescription> InputAttribute(Desc.VertexAttributes.Size());
    for (unsigned i = 0; i < Desc.VertexAttributes.Size(); i++) {
        Desc.VertexAttributes[i].WriteInto(InputAttribute[i]);
    }
    VkPipelineRasterizationStateCreateInfo RasterizerInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    };
    Desc.Rasterizer.WriteInto(RasterizerInfo);

    VkPipelineVertexInputStateCreateInfo VertexInputState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = InputBinding.Size(),
        .pVertexBindingDescriptions = InputBinding.Raw(),
        .vertexAttributeDescriptionCount = InputAttribute.Size(),
        .pVertexAttributeDescriptions = InputAttribute.Raw(),
    };
    VkPipelineInputAssemblyStateCreateInfo InputAssembly{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    VkPipelineViewportStateCreateInfo viewportState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = nullptr,
        .scissorCount = 1,
        .pScissors = nullptr,
    };

    VkPipelineColorBlendAttachmentState ColorBlendAttachment{
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };
    VkPipelineColorBlendStateCreateInfo colorBlending{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &ColorBlendAttachment,
    };

    VkPipelineMultisampleStateCreateInfo MultiSampling{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,    // TODO: Support for Multisample
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE,
    };

    VkDynamicState DynamicState[2]{
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };
    VkPipelineDynamicStateCreateInfo DynamicStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = std::size(DynamicState),
        .pDynamicStates = DynamicState,
    };

    // Dynamic rendering
    Array<VkFormat> ColorFormats(Desc.AttachmentFormats.ColorFormats.Size());
    for (unsigned i = 0; i < Desc.AttachmentFormats.ColorFormats.Size(); i++) {
        ColorFormats[i] = ImageFormatToFormat(Desc.AttachmentFormats.ColorFormats[i]);
    }
    VkPipelineRenderingCreateInfo PipelineRenderingCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .pNext = nullptr,
        .colorAttachmentCount = ColorFormats.Size(),
        .pColorAttachmentFormats = ColorFormats.Raw(),
        .depthAttachmentFormat = Desc.AttachmentFormats.DepthFormat.has_value()
                                     ? ImageFormatToFormat(Desc.AttachmentFormats.DepthFormat.value())
                                     : VK_FORMAT_UNDEFINED,
        .stencilAttachmentFormat = Desc.AttachmentFormats.StencilFormat.has_value()
                                       ? ImageFormatToFormat(Desc.AttachmentFormats.StencilFormat.value())
                                       : VK_FORMAT_UNDEFINED,

    };
    // Dynamic rendering

    VkGraphicsPipelineCreateInfo PipelineCreateInfo{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &PipelineRenderingCreateInfo,
        .flags = 0,
        .stageCount = ShaderStage.Size(),
        .pStages = ShaderStage.Raw(),
        .pVertexInputState = &VertexInputState,
        .pInputAssemblyState = &InputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &RasterizerInfo,
        .pMultisampleState = &MultiSampling,
        .pColorBlendState = &colorBlending,
        .pDynamicState = &DynamicStateCreateInfo,
        .layout = PipelineLayout,
        .renderPass = VK_NULL_HANDLE,
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
    GetConstantRangeFromShader(PushRanges, Desc.VertexShader, VK_SHADER_STAGE_VERTEX_BIT);
    GetConstantRangeFromShader(PushRanges, Desc.PixelShader, VK_SHADER_STAGE_FRAGMENT_BIT);

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
