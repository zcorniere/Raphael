#pragma once

#include "Engine/Core/RHI/Resources/RHIGraphicsPipeline.hxx"

#include "VulkanRHI/VulkanLoader.hxx"

namespace VulkanRHI
{
class VulkanShader;
class VulkanRenderPass;

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

    Ref<VulkanShader> VertexShader;
    Ref<VulkanShader> PixelShader;

    Ref<VulkanRenderPass> RenderPass;

    bool Validate() const;
    bool operator==(const GraphicsPipelineDescription&) const = default;
};

class VulkanDevice;
class VulkanShader;


class VulkanGraphicsPipeline : public RHIGraphicsPipeline
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
    VulkanDevice* Device;
    GraphicsPipelineDescription Desc;

    VkPipelineLayout PipelineLayout;
    VkPipeline VulkanPipeline;
};

}    // namespace VulkanRHI
