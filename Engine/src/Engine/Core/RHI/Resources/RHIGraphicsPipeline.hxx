#pragma once

#include "Engine/Core/RHI/RHIResource.hxx"

/// @brief Represent a shader used by the RHI
class RHIGraphicsPipeline : public RHIResource
{
public:
    RHIGraphicsPipeline(): RHIResource(RHIResourceType::GraphicsPipeline)
    {
    }
};
