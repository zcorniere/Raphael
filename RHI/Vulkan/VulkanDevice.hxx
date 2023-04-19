#pragma once

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#include "RHI/Vulkan/VulkanRHI.hxx"
#include "RHI/Vulkan/VulkanUtils.hxx"

#define VULKAN_USE_DEBUG_NAMES 1

#if VULKAN_USE_DEBUG_NAMES
    #define VULKAN_SET_DEBUG_NAME(Device, Type, Handle, Format, ...) \
        Device->SetObjectName(Type, Handle, cpplogger::fmt::format(Format, __VA_ARGS__));
#else
    #define VULKAN_SET_DEBUG_NAME(Device, Type, Handle, Format, ...)
#endif

namespace VulkanRHI
{

class VulkanQueue;
class VulkanCmdBuffer;
class VulkanMemoryManager;
class VulkanCommandBufferManager;

class VulkanDevice : public RObject
{
public:
    VulkanDevice(VkPhysicalDevice Gpu);
    ~VulkanDevice();

    void InitPhysicalDevice();
    void CreateDeviceAndQueue(const std::vector<const char *> &DeviceLayers,
                              const std::vector<const char *> &DeviceExtensions);

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
    inline const VkPhysicalDeviceProperties &GetDeviceProperties() const { return GpuProps; }
    inline const VkPhysicalDeviceLimits &GetLimits() const { return GpuProps.limits; }
    inline VulkanMemoryManager *GetMemoryManager() { return MemoryAllocator; }

    Ref<VulkanCmdBuffer> &GetCommandbuffer();

private:
    VulkanMemoryManager *MemoryAllocator;
    VulkanCommandBufferManager *CommandManager;

    VkDevice Device;
    VkPhysicalDevice Gpu;
    VkPhysicalDeviceProperties GpuProps;

    VkPhysicalDeviceFeatures PhysicalFeatures;
    std::vector<VkQueueFamilyProperties> QueueFamilyProps;

    Ref<VulkanQueue> GraphicsQueue;
    Ref<VulkanQueue> ComputeQueue;
    Ref<VulkanQueue> TransferQueue;
    Ref<VulkanQueue> PresentQueue;

    friend class VulkanDynamicRHI;
};

}    // namespace VulkanRHI