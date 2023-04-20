#include "RHI/Vulkan/VulkanRHI.hxx"

#include "RHI/Vulkan/VulkanLoader.hxx"
#include "RHI/Vulkan/VulkanUtils.hxx"

static std::string_view VulkanMessageType(const VkDebugUtilsMessageTypeFlagsEXT &s)
{
    switch (s) {
        case 7: return "General | Validation | Performance";
        case 6: return "Validation | Performance";
        case 5: return "General | Performance";
        case 4 /*VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT*/: return "Performance";
        case 3: return "General | Validation";
        case 2 /*VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT*/: return "Validation";
        case 1 /*VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT*/: return "General";
        default: return "Unknown";
    }
}

static cpplogger::Level VulkanMessageSeverityToLogLevel(const VkDebugUtilsMessageSeverityFlagBitsEXT severity)
{
    switch (severity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: return cpplogger::Level::Error;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: return cpplogger::Level::Warn;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: return cpplogger::Level::Info;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: return cpplogger::Level::Debug;
        default: return cpplogger::Level::Trace;
    }
}

static std::string_view GetMessageSeverity(const VkDebugUtilsMessageSeverityFlagBitsEXT MsgSeverity)
{
    const bool bError = (MsgSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0;
    const bool bWarning = (MsgSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) != 0;

    if (bError) {
        verify((MsgSeverity & ~VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) == 0);
        return "Error";
    } else if (bWarning) {
        verify((MsgSeverity & ~VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) == 0);
        return "Warning";
    } else if (MsgSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        verify((MsgSeverity & ~VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) == 0);
        return "Info";
    } else if (MsgSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        verify((MsgSeverity & ~VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) == 0);
        return "Verbose";
    }
    return "Unkowned";
}

static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugUtilsMessengerCallback(
    const VkDebugUtilsMessageSeverityFlagBitsEXT MsgSeverity, const VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *)
{
    const std::string_view Severity = GetMessageSeverity(MsgSeverity);

    std::string Objects;
    if (pCallbackData->objectCount) {
        Objects = cpplogger::fmt::format("\n\tObjects({}): \n", pCallbackData->objectCount);
        for (uint32_t i = 0; i < pCallbackData->objectCount; ++i) {
            const auto &object = pCallbackData->pObjects[i];
            Objects.append(cpplogger::fmt::format("\t\t- Object[{0}] name: {1}, type: {2}, handle: {3:#x}\n", i,
                                                  object.pObjectName ? object.pObjectName : "NULL",
                                                  VK_TYPE_TO_STRING(VkObjectType, object.objectType),
                                                  object.objectHandle));
        }
    }

    LOG_V(LogVulkanRHI, VulkanMessageSeverityToLogLevel(MsgSeverity), "[{}:{}({})] {}{}", Severity,
          VulkanMessageType(messageType), pCallbackData->messageIdNumber, pCallbackData->pMessage, Objects);

    if (Platform::isDebuggerPresent()) { PLATFORM_BREAK(); }
    return VK_FALSE;
}

namespace VulkanRHI
{

#if VULKAN_DEBUGGING_ENABLED

void VulkanDynamicRHI::SetupDebugLayerCallback()
{
    VkDebugUtilsMessengerCreateInfoEXT CreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = VulkanDebugUtilsMessengerCallback,
    };
    VkResult Result = VulkanAPI::vkCreateDebugUtilsMessengerEXT(m_Instance, &CreateInfo, nullptr, &Messenger);
    verify(Result == VK_SUCCESS);
}

void VulkanDynamicRHI::RemoveDebugLayerCallback()
{
    if (Messenger != VK_NULL_HANDLE) { VulkanAPI::vkDestroyDebugUtilsMessengerEXT(m_Instance, Messenger, nullptr); }
}

#endif

}    // namespace VulkanRHI
