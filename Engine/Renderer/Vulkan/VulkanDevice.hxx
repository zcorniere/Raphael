#pragma once

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#include "Engine/Renderer/Vulkan/VulkanDynamicRHI.hxx"
#include "Engine/Renderer/Vulkan/VulkanUtils.hxx"

#define VULKAN_USE_DEBUG_NAMES 1

#if VULKAN_USE_DEBUG_NAMES
    #define VULKAN_SET_DEBUG_NAME(Device, Type, Handle, Format, ...) \
        Device->SetObjectName(Type, Handle, cpplogger::fmt::format(Format, __VA_ARGS__));
#else
    #define VULKAN_SET_DEBUG_NAME(Device, Type, Handle, Format, ...)
#endif

#ifndef NDEBUG
    #include "vulkan/vk_enum_string_helper.h"
    #define VK_TYPE_TO_STRING(Type, Value) string_##Type(Value)
    #define VK_FLAGS_TO_STRING(Type, Value) string_##Type(Value).c_str()
#else
    #define VK_TYPE_TO_STRING(Type, Value) Value
    #define VK_FLAGS_TO_STRING(Type, Value) Value
#endif

class VulkanMemoryManager;
class VulkanQueue;

class VulkanDevice : public RObject
{
public:
    VulkanDevice(Ref<VulkanDynamicRHI> InRHI, VkPhysicalDevice Gpu);
    ~VulkanDevice();

    void InitGPU();
    void CreateDevice(const std::vector<const char *> &DeviceLayers, const std::vector<const char *> &DeviceExtensions);

    void PrepareForDestroy();
    void Destroy();

    void WaitUntilIdle();

    template <typename T>
    requires std::is_pointer_v<T>
    void SetObjectName(VkObjectType Type, const T Handle, const std::string Name)
    {
        VkDebugUtilsObjectNameInfoEXT NameInfo{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .objectType = Type,
            .objectHandle = (uint64)Handle,
            .pObjectName = Name.data(),
        };

        VK_CHECK_RESULT(VulkanAPI::vkSetDebugUtilsObjectNameEXT(Device, &NameInfo));
    }

    inline VkPhysicalDevice GetPhysicalHandle() const
    {
        verify(Gpu);
        return Gpu;
    }
    inline VkDevice GetInstanceHandle() const
    {
        verify(Device);
        return Device;
    }
    inline const VkPhysicalDeviceProperties &GetDeviceProperties() const
    {
        return GpuProps;
    }
    inline const VkPhysicalDeviceLimits &GetLimits() const
    {
        return GpuProps.limits;
    }
    inline Ref<VulkanMemoryManager> &GetMemoryManager()
    {
        return MemoryAllocator;
    }

private:
    Ref<VulkanDynamicRHI> RHI;
    Ref<VulkanMemoryManager> MemoryAllocator;

    VkDevice Device;
    VkPhysicalDevice Gpu;
    VkPhysicalDeviceProperties GpuProps;

    VkPhysicalDeviceFeatures PhysicalFeatures;
    std::vector<VkQueueFamilyProperties> QueueFamilyProps;

    Ref<VulkanQueue> GraphicsQueue;
    Ref<VulkanQueue> ComputeQueue;
    Ref<VulkanQueue> TransferQueue;
    Ref<VulkanQueue> PresentQueue;
};
