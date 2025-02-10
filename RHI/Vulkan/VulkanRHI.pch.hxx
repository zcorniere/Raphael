#pragma once

#define VULKAN_CUSTOM_CPU_ALLOCATOR 0

#include <Engine/Raphael.hxx>

#ifndef VK_NO_PROTOTYPES
    #define VK_NO_PROTOTYPES
#endif
#include <vulkan/vulkan.h>

#include "src/VulkanRHI/VulkanLoader.hxx"

namespace VulkanRHI
{

class FVulkanDevice;
class IDeviceChild
{
public:
    IDeviceChild() = delete;
    IDeviceChild(FVulkanDevice* InDevice): Device(InDevice)
    {
    }
    virtual ~IDeviceChild() = default;

protected:
    FVulkanDevice* const Device;
};

}    // namespace VulkanRHI
