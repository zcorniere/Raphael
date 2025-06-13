#pragma once

#include "Engine/Core/RHI/Resources/RHIGraphicsPipeline.hxx"

#include "VulkanRHI/DescriptorPoolManager.hxx"

namespace VulkanRHI
{
class RVulkanShader;

struct FGraphicsPipelineDescription
{
    struct FVertexBinding
    {
        uint32 Stride = 0;
        uint16 Binding = 0;
        VkVertexInputRate InputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        void WriteInto(VkVertexInputBindingDescription& OutState) const;

        bool operator==(const FVertexBinding& In) const = default;
    };
    TArray<FVertexBinding> VertexBindings;

    struct FVertexAttribute
    {
        uint32 Location = 0;
        uint32 Binding = 0;
        EVertexElementType Format = EVertexElementType::Float3;
        uint32 Offset = 0;

        void WriteInto(VkVertexInputAttributeDescription& OutState) const;

        bool operator==(const FVertexAttribute& In) const = default;
    };
    TArray<FVertexAttribute> VertexAttributes;

    struct FRasterizer
    {
        VkPolygonMode PolygonMode;
        VkCullModeFlags CullMode;
        VkFrontFace FrontFaceCulling;

        void WriteInto(VkPipelineRasterizationStateCreateInfo& OutState) const;

        bool operator==(const FRasterizer& In) const = default;
    };
    FRasterizer Rasterizer;

    VkPrimitiveTopology Topology;

    FRHIAttachmentFormats AttachmentFormats;

    Ref<RVulkanShader> VertexShader;
    Ref<RVulkanShader> FragmentShader;

    bool Validate() const;
    bool operator==(const FGraphicsPipelineDescription&) const = default;
};

class FVulkanDevice;
class RVulkanShader;

class RVulkanGraphicsPipeline : public RRHIGraphicsPipeline, public IDeviceChild
{
    RTTI_DECLARE_TYPEINFO(RVulkanGraphicsPipeline, RRHIGraphicsPipeline);

public:
    RVulkanGraphicsPipeline(FVulkanDevice* InDevice, const FGraphicsPipelineDescription& Description);
    ~RVulkanGraphicsPipeline();

    virtual void SetName(std::string_view Name) override;

    bool Create();
    void Bind(VkCommandBuffer CmdBuffer);

    RVulkanShader* GetShader(ERHIShaderType Type);
    RVulkanShader* GetShader(ERHIShaderType Type) const;
    TArray<WeakRef<RVulkanShader>> GetShaders() const;

    VkPipeline GetVulkanPipeline() const
    {
        return VulkanPipeline;
    }
    VkPipelineLayout GetPipelineLayout() const
    {
        return PipelineLayout;
    }

private:
    bool CreatePipelineLayout();

private:
    FGraphicsPipelineDescription Desc;

    VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
    VkPipeline VulkanPipeline = VK_NULL_HANDLE;
};

}    // namespace VulkanRHI
