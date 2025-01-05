#include "VulkanRHI/Resources/VulkanGraphicsPipeline.hxx"

#include "VulkanRHI/Resources/VulkanBuffer.hxx"
#include "VulkanRHI/Resources/VulkanShader.hxx"
#include "VulkanRHI/VulkanDevice.hxx"
#include "VulkanRHI/VulkanLoader.hxx"

namespace VulkanRHI
{

void FGraphicsPipelineDescription::FVertexBinding::WriteInto(VkVertexInputBindingDescription& OutState) const
{
    OutState.binding = Binding;
    OutState.inputRate = (VkVertexInputRate)InputRate;
    OutState.stride = Stride;
}

void FGraphicsPipelineDescription::FVertexAttribute::WriteInto(VkVertexInputAttributeDescription& OutState) const
{
    OutState.binding = Binding;
    OutState.format = VertexElementToFormat(Format);
    OutState.location = Location;
    OutState.offset = Offset;
}

void FGraphicsPipelineDescription::FRasterizer::WriteInto(VkPipelineRasterizationStateCreateInfo& OutState) const
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

bool FGraphicsPipelineDescription::Validate() const
{
    // Vertex shader is required
    if (!VertexShader.IsValid())
        return false;
    // Fragment and vertex shader must be different
    if (VertexShader == FragmentShader)
        return false;
    // If there is a Fragment shader, it must be valid
    if (FragmentShader != nullptr && !FragmentShader.IsValid())
        return false;

    return true;
}

RVulkanGraphicsPipeline::RVulkanGraphicsPipeline(FVulkanDevice* InDevice,
                                                 const FGraphicsPipelineDescription& Description)
    : IDeviceChild(InDevice), Desc(Description), DescriptorManager(InDevice)
{
    Create();
}

RVulkanGraphicsPipeline::~RVulkanGraphicsPipeline()
{
    RHI::RHIWaitUntilIdle();
    DescriptorManager.Destroy();

    if (VulkanPipeline) {
        VulkanAPI::vkDestroyPipeline(Device->GetHandle(), VulkanPipeline, VULKAN_CPU_ALLOCATOR);
    }
    if (PipelineLayout) {
        VulkanAPI::vkDestroyPipelineLayout(Device->GetHandle(), PipelineLayout, VULKAN_CPU_ALLOCATOR);
    }
}

void RVulkanGraphicsPipeline::SetName(std::string_view Name)
{
    RObject::SetName(Name);
    if (VulkanPipeline) {
        VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_PIPELINE, VulkanPipeline, "{:s}", Name);
    }
    if (PipelineLayout) {
        VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_PIPELINE_LAYOUT, PipelineLayout, "{:s}.PipelineLayout", Name);
    }
}

void RVulkanGraphicsPipeline::SetInput(std::string_view Name, const Ref<RRHIBuffer>& Buffer)
{
    Ref<RVulkanBuffer> VulkanBuffer = Buffer.As<RVulkanBuffer>();
    DescriptorManager.SetInput(Name, VulkanBuffer);
}

bool RVulkanGraphicsPipeline::Create()
{
    auto FillShaderStageInfo = [](Ref<RVulkanShader>& InShader,
                                  TArray<VkPipelineShaderStageCreateInfo>& OutShaderStage) {
        OutShaderStage.Add(VkPipelineShaderStageCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = &InShader->GetShaderModuleCreateInfo(),
            .stage = ConvertToVulkanType(InShader->GetShaderType()),
            .module = VK_NULL_HANDLE,
            .pName = InShader->GetEntryPoint(),
        });
    };
    CreatePipelineLayout();

    TArray<VkPipelineShaderStageCreateInfo> ShaderStage;
    FillShaderStageInfo(Desc.VertexShader, ShaderStage);
    FillShaderStageInfo(Desc.FragmentShader, ShaderStage);

    TArray<VkVertexInputBindingDescription> InputBinding(Desc.VertexBindings.Size());
    for (unsigned i = 0; i < Desc.VertexBindings.Size(); i++) {
        Desc.VertexBindings[i].WriteInto(InputBinding[i]);
    }
    TArray<VkVertexInputAttributeDescription> InputAttribute(Desc.VertexAttributes.Size());
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
    TArray<VkFormat> ColorFormats(Desc.AttachmentFormats.ColorFormats.Size());
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
    // End of Dynamic rendering

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

void RVulkanGraphicsPipeline::Bind(VkCommandBuffer CmdBuffer)
{
    VulkanAPI::vkCmdBindPipeline(CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, VulkanPipeline);
    DescriptorManager.Bind(CmdBuffer, PipelineLayout, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

RVulkanShader* RVulkanGraphicsPipeline::GetShader(ERHIShaderType Type)
{
    switch (Type) {
        case ERHIShaderType::Vertex:
            return Desc.VertexShader.AsRaw<RVulkanShader>();
        case ERHIShaderType::Fragment:
            return Desc.FragmentShader.AsRaw<RVulkanShader>();
        case ERHIShaderType::Compute: {
            checkNoEntry();
            return nullptr;
        }
    }
    checkNoEntry();
    return nullptr;
}
RVulkanShader* RVulkanGraphicsPipeline::GetShader(ERHIShaderType Type) const
{
    return const_cast<RVulkanGraphicsPipeline*>(this)->GetShader(Type);
}

static bool GetConstantRangeFromShader(TArray<VkPushConstantRange>& OutPushRanges, Ref<RVulkanShader>& InShader,
                                       const VkShaderStageFlags ShaderStage)
{
    if (!InShader->GetReflectionData().PushConstants.has_value())
        return true;

    VkPushConstantRange NewRange{
        .stageFlags = ShaderStage,
        .offset = InShader->GetReflectionData().PushConstants->Offset,
        .size = InShader->GetReflectionData().PushConstants->Size,
    };
    for (VkPushConstantRange& Range: OutPushRanges) {
        // The same range is used in multiple ranges, so just add the ShaderStage and return
        if (Range.size == NewRange.size && Range.offset == NewRange.offset) {
            Range.stageFlags |= ShaderStage;
            return true;
        }
    }
    OutPushRanges.Emplace(NewRange);
    return true;
}

bool RVulkanGraphicsPipeline::CreatePipelineLayout()
{
    TArray<VkPushConstantRange> PushRanges;
    GetConstantRangeFromShader(PushRanges, Desc.VertexShader, VK_SHADER_STAGE_VERTEX_BIT);
    GetConstantRangeFromShader(PushRanges, Desc.FragmentShader, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreateDescriptorSetLayout();

    VkPipelineLayoutCreateInfo CreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .setLayoutCount = DescriptorManager.GetDescriptorSetLayout().Size(),
        .pSetLayouts = DescriptorManager.GetDescriptorSetLayout().Raw(),
        .pushConstantRangeCount = PushRanges.Size(),
        .pPushConstantRanges = PushRanges.Raw(),
    };
    VK_CHECK_RESULT(
        VulkanAPI::vkCreatePipelineLayout(Device->GetHandle(), &CreateInfo, VULKAN_CPU_ALLOCATOR, &PipelineLayout));
    return true;
}

bool RVulkanGraphicsPipeline::CreateDescriptorSetLayout()
{
    TArray<Ref<RVulkanShader>> Shaders{Desc.VertexShader, Desc.FragmentShader};
    return DescriptorManager.Initialize(Shaders, 1);
}

}    // namespace VulkanRHI
