#pragma once

#include "Engine/Core/RHI/Resources/RHIGraphicsPipeline.hxx"

#include "VulkanRHI/DescriptorPoolManager.hxx"

namespace VulkanRHI
{
class RVulkanShader;

struct FGraphicsPipelineDescription {
    struct FVertexBinding {
        uint32 Stride;
        uint16 Binding;
        uint16 InputRate;

        void WriteInto(VkVertexInputBindingDescription& OutState) const;

        bool operator==(const FVertexBinding& In) const = default;
    };
    TArray<FVertexBinding> VertexBindings;

    struct FVertexAttribute {
        uint32 Location;
        uint32 Binding;
        EVertexElementType Format;
        uint32 Offset;

        void WriteInto(VkVertexInputAttributeDescription& OutState) const;

        bool operator==(const FVertexAttribute& In) const = default;
    };
    TArray<FVertexAttribute> VertexAttributes;

    struct FRasterizer {
        VkPolygonMode PolygonMode;
        VkCullModeFlags CullMode;
        VkFrontFace FrontFaceCulling;

        void WriteInto(VkPipelineRasterizationStateCreateInfo& OutState) const;

        bool operator==(const FRasterizer& In) const = default;
    };
    FRasterizer Rasterizer;

    FRHIAttachmentFormats AttachmentFormats;

    Ref<RVulkanShader> VertexShader;
    Ref<RVulkanShader> PixelShader;

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

    virtual void SetInput(std::string_view Name, const Ref<RRHIBuffer>& Buffer) override;

    bool Create();
    void Bind(VkCommandBuffer CmdBuffer);

    RVulkanShader* GetShader(ERHIShaderType Type);
    RVulkanShader* GetShader(ERHIShaderType Type) const;

    VkPipeline GetVulkanPipeline() const
    {
        return VulkanPipeline;
    }

private:
    bool CreatePipelineLayout();
    bool CreateDescriptorSetLayout();

private:
    FGraphicsPipelineDescription Desc;

    FDescriptorSetManager DescriptorManager;

    VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
    VkPipeline VulkanPipeline = VK_NULL_HANDLE;
};

}    // namespace VulkanRHI
