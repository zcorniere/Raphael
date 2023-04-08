#pragma once

#include "Engine/Renderer/RHI/RHIResource.hxx"

#include "Engine/Renderer/Vulkan/VulkanLoader.hxx"

namespace VulkanRHI
{

class VulkanFramebuffer : public RHIFramebuffer
{
public:
    VulkanFramebuffer(FramebufferSpecification &Specs);
};

}    // namespace VulkanRHI
