#include "Engine/Renderer/Vulkan/VulkanPhysicalDevice.hxx"

#include "Engine/Renderer/Vulkan/VulkanContext.hxx"

namespace Raphael
{
DECLARE_LOGGER_CATEGORY(Core, LogVulkanPhysicalDevice, Info);

VulkanPhysicalDevice::VulkanPhysicalDevice()
{
    auto vkInstance = VulkanContext::GetInstance();

    uint32_t gpuCount = 0;
    // Get number of available physical devices
    vkEnumeratePhysicalDevices(vkInstance, &gpuCount, nullptr);
    check(gpuCount > 0);
    // Enumerate devices
    std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
    VK_CHECK_RESULT(vkEnumeratePhysicalDevices(vkInstance, &gpuCount, physicalDevices.data()));

    VkPhysicalDevice selectedPhysicalDevice = nullptr;
    for (VkPhysicalDevice physicalDevice: physicalDevices) {
        vkGetPhysicalDeviceProperties(physicalDevice, &m_Properties);
        if (m_Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            selectedPhysicalDevice = physicalDevice;
            break;
        }
    }

    if (!selectedPhysicalDevice) {
        LOG(LogVulkanPhysicalDevice, Trace, "Could not find discrete GPU.");
        selectedPhysicalDevice = physicalDevices.back();
    }

    checkMsg(selectedPhysicalDevice, "Could not find any physical devices!");
    m_PhysicalDevice = selectedPhysicalDevice;

    vkGetPhysicalDeviceFeatures(m_PhysicalDevice, &m_Features);
    vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &m_MemoryProperties);

    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, nullptr);
    check(queueFamilyCount > 0);
    m_QueueFamilyProperties.resize(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, m_QueueFamilyProperties.data());

    uint32_t extCount = 0;
    vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extCount, nullptr);
    if (extCount > 0) {
        std::vector<VkExtensionProperties> extensions(extCount);
        if (vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extCount, extensions.data()) ==
            VK_SUCCESS) {
            LOG(LogVulkanPhysicalDevice, Trace, "Selected physical device has {0} extensions", extensions.size());
            for (const auto &ext: extensions) {
                m_SupportedExtensions.emplace(ext.extensionName);
                LOG(LogVulkanPhysicalDevice, Trace, "  {0}", ext.extensionName);
            }
        }
    }

    // Queue families
    // Desired queues need to be requested upon logical device creation
    // Due to differing queue family configurations of Vulkan implementations this can be a bit tricky, especially if
    // the application requests different queue types

    // Get queue family indices for the requested queue family types
    // Note that the indices may overlap depending on the implementation

    static const float defaultQueuePriority(0.0f);

    int requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
    m_QueueFamilyIndices = GetQueueFamilyIndices(requestedQueueTypes);

    // Graphics queue
    if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT) {
        VkDeviceQueueCreateInfo queueInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = static_cast<std::uint32_t>(m_QueueFamilyIndices.Graphics),
            .queueCount = 1,
            .pQueuePriorities = &defaultQueuePriority,
        };
        m_QueueCreateInfos.push_back(queueInfo);
    }

    // Dedicated compute queue
    if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT) {
        if (m_QueueFamilyIndices.Compute != m_QueueFamilyIndices.Graphics) {
            // If compute family index differs, we need an additional queue create info for the compute queue
            VkDeviceQueueCreateInfo queueInfo{
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = static_cast<std::uint32_t>(m_QueueFamilyIndices.Compute),
                .queueCount = 1,
                .pQueuePriorities = &defaultQueuePriority,
            };
            m_QueueCreateInfos.push_back(queueInfo);
        }
    }

    // Dedicated transfer queue
    if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT) {
        if ((m_QueueFamilyIndices.Transfer != m_QueueFamilyIndices.Graphics) &&
            (m_QueueFamilyIndices.Transfer != m_QueueFamilyIndices.Compute)) {
            // If compute family index differs, we need an additional queue create info for the compute queue
            VkDeviceQueueCreateInfo queueInfo{
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = static_cast<std::uint32_t>(m_QueueFamilyIndices.Transfer),
                .queueCount = 1,
                .pQueuePriorities = &defaultQueuePriority,
            };
            m_QueueCreateInfos.push_back(queueInfo);
        }
    }

    m_DepthFormat = FindDepthFormat();
    check(m_DepthFormat);
}

VulkanPhysicalDevice::~VulkanPhysicalDevice()
{
}

bool VulkanPhysicalDevice::IsExtensionSupported(const std::string &extensionName) const
{
    return m_SupportedExtensions.find(extensionName) != m_SupportedExtensions.end();
}

std::uint32_t VulkanPhysicalDevice::GetMemoryTypeIndex(std::uint32_t typeBits, VkMemoryPropertyFlags properties) const
{
    // Iterate over all memory types available for the device used in this example
    for (std::uint32_t i = 0; i < m_MemoryProperties.memoryTypeCount; i++) {
        if ((typeBits & 1) == 1) {
            if ((m_MemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) return i;
        }
        typeBits >>= 1;
    }

    LOG(LogVulkanPhysicalDevice, Fatal, "Could not find a suitable memory type!");
    checkNoEntry();
    return std::numeric_limits<std::uint32_t>::max();
}

Ref<VulkanPhysicalDevice> VulkanPhysicalDevice::Select()
{
    return Ref<VulkanPhysicalDevice>::Create();
}

VkFormat VulkanPhysicalDevice::FindDepthFormat() const
{
    static const std::vector<VkFormat> DepthFormats = {
        VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM_S8_UINT,  VK_FORMAT_D16_UNORM,
    };

    for (auto &Format: DepthFormats) {
        VkFormatProperties formatProps;
        vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, Format, &formatProps);
        if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) return Format;
    }
    return VK_FORMAT_UNDEFINED;
}

VulkanPhysicalDevice::QueueFamilyIndices VulkanPhysicalDevice::GetQueueFamilyIndices(int flags)
{
    QueueFamilyIndices indices;

    // Dedicated queue for compute
    // Try to find a queue family index that supports compute but not graphics
    if (flags & VK_QUEUE_COMPUTE_BIT) {
        for (uint32_t i = 0; i < m_QueueFamilyProperties.size(); i++) {
            auto &queueFamilyProperties = m_QueueFamilyProperties[i];
            if ((queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT) &&
                ((queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)) {
                indices.Compute = i;
                break;
            }
        }
    }

    // Dedicated queue for transfer
    // Try to find a queue family index that supports transfer but not graphics and compute
    if (flags & VK_QUEUE_TRANSFER_BIT) {
        for (uint32_t i = 0; i < m_QueueFamilyProperties.size(); i++) {
            auto &queueFamilyProperties = m_QueueFamilyProperties[i];
            if ((queueFamilyProperties.queueFlags & VK_QUEUE_TRANSFER_BIT) &&
                ((queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) &&
                ((queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT) == 0)) {
                indices.Transfer = i;
                break;
            }
        }
    }

    // For other queue types or if no separate compute queue is present, return the first one to support the requested
    // flags
    for (uint32_t i = 0; i < m_QueueFamilyProperties.size(); i++) {
        if ((flags & VK_QUEUE_TRANSFER_BIT) && indices.Transfer == -1) {
            if (m_QueueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) indices.Transfer = i;
        }

        if ((flags & VK_QUEUE_COMPUTE_BIT) && indices.Compute == -1) {
            if (m_QueueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) indices.Compute = i;
        }

        if (flags & VK_QUEUE_GRAPHICS_BIT) {
            if (m_QueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) indices.Graphics = i;
        }
    }

    return indices;
}

}    // namespace Raphael
