#pragma once

namespace VulkanRHI
{

#if VULKAN_DEBUGGING_ENABLED

class VulkanRHI_Debug
{
public:
    TArray<const char*> GetSupportedInstanceLayers();
    void SetupDebugLayer(VkInstance Instance);
    void RemoveDebugLayer(VkInstance Instance);

    bool IsValidationLayersMissing() const
    {
        return bValidationLayersAreMissing;
    }

private:
    bool bValidationLayersAreMissing = false;
    VkDebugUtilsMessengerEXT Messenger = VK_NULL_HANDLE;
};

#endif    // VULKAN_DEBUGGING_ENABLED

}    // namespace VulkanRHI
