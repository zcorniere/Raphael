#pragma once

#include "Engine/Renderer/RHI/IDynamicRHI.hxx"

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

DECLARE_LOGGER_CATEGORY(Core, LogVulkanRHI, Info);

namespace VulkanRHI
{

class IVulkanDynamicRHI : public IDynamicRHI
{
public:
    IVulkanDynamicRHI()
    {
        SetName("Vulkan");
    }

    RHIInterfaceType GetInterfaceType() const override
    {
        return RHIInterfaceType::Vulkan;
    }

    virtual VkInstance RHIGetVkInstance() const = 0;
    virtual VkDevice RHIGetVkDevice() const = 0;
    virtual VkPhysicalDevice RHIGetVkPhysicalDevice() const = 0;
};

FORCEINLINE Ref<IVulkanDynamicRHI> GetIVulkanDynamicRHI()
{
    checkMsg(GDynamicRHI, "Tried to fetch RHI too early");
    check(GDynamicRHI->GetInterfaceType() == RHIInterfaceType::Vulkan);
    return GetDynamicRHI<IVulkanDynamicRHI>();
}

}    // namespace VulkanRHI
