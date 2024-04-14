#pragma once

#include "Engine/Core/RHI/Resources/RHIGraphicsPipeline.hxx"

#include "VulkanRHI/VulkanLoader.hxx"

namespace VulkanRHI
{
class VulkanShader;

struct GraphicsPipelineDescription {
    struct VertexBinding {
        uint32 Stride;
        uint16 Binding;
        uint16 InputRate;

        void WriteInto(VkVertexInputBindingDescription& OutState) const;

        bool operator==(const VertexBinding& In) const = default;
    };
    Array<VertexBinding> VertexBindings;

    struct VertexAttribute {
        uint32 Location;
        uint32 Binding;
        EVertexElementType Format;
        uint32 Offset;

        void WriteInto(VkVertexInputAttributeDescription& OutState) const;

        bool operator==(const VertexAttribute& In) const = default;
    };
    Array<VertexAttribute> VertexAttributes;

    struct Rasterizer {
        VkPolygonMode PolygonMode;
        VkCullModeFlags CullMode;
        VkFrontFace FrontFaceCulling;

        void WriteInto(VkPipelineRasterizationStateCreateInfo& OutState) const;

        bool operator==(const Rasterizer& In) const = default;
    };
    Rasterizer Rasterizer;

    struct AttachmentFormats {
        Array<EImageFormat> ColorFormats;
        std::optional<EImageFormat> DepthFormat = std::nullopt;
        std::optional<EImageFormat> StencilFormat = std::nullopt;

        bool operator==(const AttachmentFormats& In) const = default;
    };
    AttachmentFormats AttachmentFormats;

    Ref<VulkanShader> VertexShader;
    Ref<VulkanShader> PixelShader;

    bool Validate() const;
    bool operator==(const GraphicsPipelineDescription&) const = default;
};

class VulkanDevice;
class VulkanShader;

class VulkanGraphicsPipeline : public RHIGraphicsPipeline, public IDeviceChild
{
public:
    VulkanGraphicsPipeline(VulkanDevice* InDevice, const GraphicsPipelineDescription& Description);
    ~VulkanGraphicsPipeline();

    void SetName(std::string_view Name) override;

    bool Create();
    void Bind(VkCommandBuffer CmdBuffer)
    {
        VulkanAPI::vkCmdBindPipeline(CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, VulkanPipeline);
    }

    VkPipeline GetVulkanPipeline() const
    {
        return VulkanPipeline;
    }

private:
    bool CreatePipelineLayout();

private:
    GraphicsPipelineDescription Desc;

    VkPipelineLayout PipelineLayout;
    VkPipeline VulkanPipeline;
};

}    // namespace VulkanRHI
