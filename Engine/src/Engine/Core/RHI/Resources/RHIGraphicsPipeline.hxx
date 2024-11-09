#pragma once

#include "Engine/Core/RHI/RHIDefinitions.hxx"
#include "Engine/Core/RHI/RHIResource.hxx"

struct FRHIAttachmentFormats {
    TArray<EImageFormat> ColorFormats;
    std::optional<EImageFormat> DepthFormat = std::nullopt;
    std::optional<EImageFormat> StencilFormat = std::nullopt;

    bool operator==(const FRHIAttachmentFormats&) const = default;
};

struct FRHIGraphicsPipelineSpecification {
    std::string VertexShader;
    std::string PixelShader;

    struct FRasterizerDesc {
        EPolygonMode PolygonMode;
        ECullMode CullMode;
        EFrontFace FrontFaceCulling;

        bool operator==(const FRasterizerDesc&) const = default;
    };
    FRasterizerDesc Rasterizer;

    FRHIAttachmentFormats AttachmentFormats;

    bool operator==(const FRHIGraphicsPipelineSpecification&) const = default;
};

/// @brief Represent a shader used by the RHI
class RRHIGraphicsPipeline : public RRHIResource
{
    RTTI_DECLARE_TYPEINFO(RRHIGraphicsPipeline, RRHIResource);

public:
    RRHIGraphicsPipeline(): RRHIResource(ERHIResourceType::GraphicsPipeline)
    {
    }

    virtual ~RRHIGraphicsPipeline() = default;
};
