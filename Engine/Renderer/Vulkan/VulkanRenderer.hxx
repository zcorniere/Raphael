#pragma once

#include "Engine/Renderer/RendererAPI.hxx"

namespace Raphael
{

class VulkanRenderer : public RendererAPI
{
    virtual void Init() override;
    virtual void Shutdown() override;

    virtual const RendererCapabilities &GetCapabilities() const override;
};

}    // namespace Raphael
