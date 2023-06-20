#pragma once

#include "Engine/Core/RHI/Resources/RHIGraphicsPipeline.hxx"

#include "VulkanRHI/VulkanLoader.hxx"

namespace VulkanRHI
{

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

    std::string VertexShader;
    std::string PixelShader;

    struct RenderTargets {
        struct AttachmentRef {
            uint32 Attachment;
            VkImageLayout Layout;

            void WriteInto(VkAttachmentReference& OutState) const;

            bool operator==(const AttachmentRef& In) const = default;
        };

        Array<AttachmentRef> ColorAttachments;
        Array<AttachmentRef> ResolveAttachments;
        AttachmentRef Depth;

        struct AttachmentDesc {
            EImageFormat Format;
            uint8 Flags;
            VkAttachmentLoadOp LoadOp;
            VkAttachmentStoreOp StoreOp;
            VkImageLayout InitialLayout;
            VkImageLayout FinalLayout;

            bool operator==(const AttachmentDesc& In) const = default;

            void WriteInto(VkAttachmentDescription& OutState) const;
        };
        Array<AttachmentDesc> Descriptions;
    };

    bool operator==(const GraphicsPipelineDescription&) const = default;
};

class VulkanDevice;
class VulkanShader;


class VulkanGraphicsPipeline : public RHIGraphicsPipeline
{
public:
    VulkanGraphicsPipeline(Ref<VulkanDevice>& InDevice, const GraphicsPipelineDescription& Description);
    ~VulkanGraphicsPipeline();

    bool Create(bool bForceRecompileShaders = false);
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
    Ref<VulkanDevice> Device;
    GraphicsPipelineDescription Desc;

    Ref<VulkanShader> Shaders[2];

    VkPipelineLayout PipelineLayout;
    VkPipeline VulkanPipeline;
};

}    // namespace VulkanRHI
