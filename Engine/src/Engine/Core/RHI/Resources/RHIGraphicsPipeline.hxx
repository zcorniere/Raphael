#pragma once

#include "Engine/Core/RHI/RHIResource.hxx"

#include "Engine/Core/RHI/RHIDefinitions.hxx"

class RHIShader;

struct RHIGraphicsPipelineInitializer {

    std::string VertexShader;
    std::string PixelShader;

    struct RasterizerDesc {
        EPolygonMode PolygonMode;
        ECullMode CullMode;
        EFrontFace FrontFaceCulling;
    };
    RasterizerDesc Rasterizer;
};

/// @brief Represent a shader used by the RHI
class RHIGraphicsPipeline : public RHIResource
{
public:
    RHIGraphicsPipeline(): RHIResource(RHIResourceType::GraphicsPipeline)
    {
    }
};
