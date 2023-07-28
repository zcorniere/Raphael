#pragma once

#include "Engine/Core/RHI/RHIResource.hxx"

#include "Engine/Core/RHI/RHIDefinitions.hxx"

struct RHIGraphicsPipelineInitializer {

    std::string VertexShader;
    std::string PixelShader;

    struct RasterizerDesc {
        EPolygonMode PolygonMode;
        ECullMode CullMode;
        EFrontFace FrontFaceCulling;
    };
    RasterizerDesc Rasterizer;
    RHIRenderPassDescription RenderPass;
};

/// @brief Represent a shader used by the RHI
class RHIGraphicsPipeline : public RHIResource
{
public:
    RHIGraphicsPipeline(): RHIResource(RHIResourceType::GraphicsPipeline)
    {
    }
};
