#pragma once

#include "Engine/Core/RHI/RHIResource.hxx"

#include "RHI/Vulkan/VulkanLoader.hxx"

namespace VulkanRHI
{

class VulkanFramebuffer : public RHIFramebuffer
{
public:
    VulkanFramebuffer(FramebufferSpecification &Specs);
};

}    // namespace VulkanRHI
