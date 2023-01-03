#include "Engine/Renderer/Vulkan/VulkanDevice.hxx"

#include "Engine/Renderer/Vulkan/VulkanContext.hxx"

namespace Raphael
{

DECLARE_LOGGER_CATEGORY(Core, LogVulkanDevice, Info);

// Vulkan Device
VulkanDevice::VulkanDevice(const Ref<VulkanPhysicalDevice> &physicalDevice, VkPhysicalDeviceFeatures enabledFeatures)
    : m_PhysicalDevice(physicalDevice), m_EnabledFeatures(enabledFeatures)
{
    // Do we need to enable any other extensions (eg. NV_RAYTRACING?)
    std::vector<const char *> deviceExtensions;
    // If the device will be used for presenting to a display via a swapchain we need to request the swapchain extension
    check(m_PhysicalDevice->IsExtensionSupported(VK_KHR_SWAPCHAIN_EXTENSION_NAME));
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    if (m_PhysicalDevice->IsExtensionSupported(VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME))
        deviceExtensions.push_back(VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME);
    if (m_PhysicalDevice->IsExtensionSupported(VK_NV_DEVICE_DIAGNOSTICS_CONFIG_EXTENSION_NAME))
        deviceExtensions.push_back(VK_NV_DEVICE_DIAGNOSTICS_CONFIG_EXTENSION_NAME);

    VkDeviceCreateInfo deviceCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = static_cast<uint32_t>(physicalDevice->m_QueueCreateInfos.size()),
        .pQueueCreateInfos = physicalDevice->m_QueueCreateInfos.data(),
        .pEnabledFeatures = &enabledFeatures,
    };

    // If a pNext(Chain) has been passed, we need to add it to the device creation info
    // VkPhysicalDeviceFeatures2 physicalDeviceFeatures2{};

    // Enable the debug marker extension if it is present (likely meaning a debugging tool is present)
    if (m_PhysicalDevice->IsExtensionSupported(VK_EXT_DEBUG_MARKER_EXTENSION_NAME)) {
        deviceExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
        m_EnableDebugMarkers = true;
    }

    if (deviceExtensions.size() > 0) {
        deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
        deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
    }

    VK_CHECK_RESULT(
        vkCreateDevice(m_PhysicalDevice->GetVulkanPhysicalDevice(), &deviceCreateInfo, nullptr, &m_LogicalDevice));

    // Get a graphics queue from the device
    vkGetDeviceQueue(m_LogicalDevice, m_PhysicalDevice->m_QueueFamilyIndices.Graphics, 0, &m_GraphicsQueue);
    vkGetDeviceQueue(m_LogicalDevice, m_PhysicalDevice->m_QueueFamilyIndices.Compute, 0, &m_ComputeQueue);
}

VulkanDevice::~VulkanDevice()
{
}

void VulkanDevice::Destroy()
{
    vkDeviceWaitIdle(m_LogicalDevice);
    vkDestroyDevice(m_LogicalDevice, nullptr);
}

}    // namespace Raphael
