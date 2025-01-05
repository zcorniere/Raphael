#pragma once

#include "VulkanRHI/VulkanCommandContext.hxx"
#include "VulkanRHI/VulkanPlatform.hxx"
#include "VulkanRHI/VulkanRHI.hxx"
#include "VulkanRHI/VulkanUtils.hxx"

#if VULKAN_DEBUGGING_ENABLED
    #define VULKAN_SET_DEBUG_NAME(Device, Type, Handle, Format, ...) \
        Device->SetObjectName(Type, Handle, std::format(Format, ##__VA_ARGS__));
#else
    #define VULKAN_SET_DEBUG_NAME(Device, Type, Handle, Format, ...)
#endif

namespace VulkanRHI
{

class FVulkanQueue;
class FVulkanCmdBuffer;
class FVulkanMemoryManager;
class VulkanCommandBufferManager;

class FVulkanDevice : public FNamedClass
{
public:
    explicit FVulkanDevice(VkPhysicalDevice Gpu);
    ~FVulkanDevice();

    virtual void SetName(std::string_view InName) override final;

    virtual std::string_view GetDeviceName() const;

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
    inline FVulkanMemoryManager* GetMemoryManager()
    {
        check(MemoryAllocator);
        return MemoryAllocator.get();
    }

    FVulkanCommandContext* GetImmediateContext() const
    {
        return ImmediateContext;
    }

private:
    void Destroy();
    bool CreateDeviceAndQueue(const TArray<const char*>& DeviceLayers,
                              const FVulkanDeviceExtensionArray& DeviceExtensions);

public:
    std::unique_ptr<FVulkanQueue> GraphicsQueue;
    std::unique_ptr<FVulkanQueue> ComputeQueue;
    std::unique_ptr<FVulkanQueue> TransferQueue;
    // Present queue is not a dedicated object, it's just a reference to one of the above queues
    FVulkanQueue* PresentQueue = nullptr;

    FVulkanCommandContext* ImmediateContext = nullptr;

private:
    std::unique_ptr<FVulkanMemoryManager> MemoryAllocator;

    VkDevice Device = VK_NULL_HANDLE;
    VkPhysicalDevice Gpu = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties GpuProps;

    VkPhysicalDeviceFeatures PhysicalFeatures;
    TArray<VkQueueFamilyProperties> QueueFamilyProps;

    friend class FVulkanDynamicRHI;
};

}    // namespace VulkanRHI
