#pragma once

#include "vulkan/vulkan_core.h"
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#include "VulkanRHI/VulkanRHI.hxx"
#include "VulkanRHI/VulkanUtils.hxx"

#include "Engine/Core/Memory/SmartPointers.hxx"

#if VULKAN_DEBUGGING_ENABLED
    #define VULKAN_SET_DEBUG_NAME(Device, Type, Handle, Format, ...) \
        Device->SetObjectName(Type, Handle, std::format(Format, ##__VA_ARGS__));
#else
    #define VULKAN_SET_DEBUG_NAME(Device, Type, Handle, Format, ...)
#endif

namespace VulkanRHI
{

class VulkanQueue;
class VulkanCmdBuffer;
class VulkanMemoryManager;
class VulkanCommandBufferManager;

class VulkanDevice : public NamedClass
{
public:
    explicit VulkanDevice(VkPhysicalDevice Gpu);
    ~VulkanDevice();

    virtual void SetName(std::string_view InName) override final;

    void InitPhysicalDevice();
    void SetupPresentQueue(VkSurfaceKHR Surface);

    void WaitUntilIdle();

#if VULKAN_DEBUGGING_ENABLED
    template <typename T>
    void SetObjectName(VkObjectType Type, const T Handle, const std::string& Name) const
    {
        VkDebugUtilsObjectNameInfoEXT NameInfo{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = Type,
            .objectHandle = (uint64)Handle,
            .pObjectName = (Name.empty()) ? (nullptr) : (Name.data()),
        };

        VK_CHECK_RESULT(VulkanAPI::vkSetDebugUtilsObjectNameEXT(Device, &NameInfo));
    }
#endif

    inline VkPhysicalDevice GetPhysicalHandle() const
    {
        ensure(Gpu);
        return Gpu;
    }
    inline VkDevice GetHandle() const
    {
        ensure(Device);
        return Device;
    }
    inline const VkPhysicalDeviceProperties& GetDeviceProperties() const
    {
        return GpuProps;
    }
    inline const VkPhysicalDeviceLimits& GetLimits() const
    {
        return GpuProps.limits;
    }
    inline VulkanMemoryManager* GetMemoryManager()
    {
        check(MemoryAllocator);
        return MemoryAllocator.Get();
    }

    inline VulkanCommandBufferManager* GetCommandManager()
    {
        check(CommandManager);
        return CommandManager.Get();
    }

private:
    void Destroy();
    void CreateDeviceAndQueue(const Array<const char*>& DeviceLayers, const Array<const char*>& DeviceExtensions);

public:
    UniquePtr<VulkanQueue> GraphicsQueue = nullptr;
    UniquePtr<VulkanQueue> ComputeQueue = nullptr;
    UniquePtr<VulkanQueue> TransferQueue = nullptr;
    VulkanQueue* PresentQueue = nullptr;

private:
    UniquePtr<VulkanMemoryManager> MemoryAllocator = nullptr;
    UniquePtr<VulkanCommandBufferManager> CommandManager = nullptr;

    VkDevice Device = VK_NULL_HANDLE;
    VkPhysicalDevice Gpu = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties GpuProps;

    VkPhysicalDeviceFeatures PhysicalFeatures;
    Array<VkQueueFamilyProperties> QueueFamilyProps;

    friend class VulkanDynamicRHI;
};

}    // namespace VulkanRHI
