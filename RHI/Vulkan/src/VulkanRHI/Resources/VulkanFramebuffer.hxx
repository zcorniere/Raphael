#pragma once

#include "Engine/Core/RHI/RHIResource.hxx"

#include "VulkanRHI/VulkanLoader.hxx"

namespace VulkanRHI
{

class VulkanFramebuffer : public RHIFramebuffer
{
public:
    VulkanFramebuffer(FramebufferSpecification &Specs);
};

}    // namespace VulkanRHI
