#pragma once

#include "Engine/Core/RHI/RHIDefinitions.hxx"
#include "Engine/Core/RHI/RHIResource.hxx"

struct RHIAttachmentFormats {
    Array<EImageFormat> ColorFormats;
    std::optional<EImageFormat> DepthFormat = std::nullopt;
    std::optional<EImageFormat> StencilFormat = std::nullopt;

    bool operator==(const RHIAttachmentFormats&) const = default;
};

struct RHIGraphicsPipelineSpecification {
    std::string VertexShader;
    std::string PixelShader;

    struct RasterizerDesc {
        EPolygonMode PolygonMode;
        ECullMode CullMode;
        EFrontFace FrontFaceCulling;

        bool operator==(const RasterizerDesc&) const = default;
    };
    RasterizerDesc Rasterizer;

    RHIAttachmentFormats AttachmentFormats;

    bool operator==(const RHIGraphicsPipelineSpecification&) const = default;
};

/// @brief Represent a shader used by the RHI
class RHIGraphicsPipeline : public RHIResource
{
public:
    RHIGraphicsPipeline(): RHIResource(ERHIResourceType::GraphicsPipeline)
    {
    }

    virtual ~RHIGraphicsPipeline() = default;
};
