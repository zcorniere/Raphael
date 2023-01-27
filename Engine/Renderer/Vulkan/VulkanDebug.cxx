#include "Engine/Renderer/Vulkan/VulkanDynamicRHI.hxx"

#include "Engine/Renderer/Vulkan/VulkanLoader.hxx"
#include "Engine/Renderer/Vulkan/VulkanUtils.hxx"

namespace Raphael::RHI
{

constexpr static std::string_view VulkanMessageType(const VkDebugUtilsMessageTypeFlagsEXT &s)
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

constexpr cpplogger::Level VulkanMessageSeverityToLogLevel(const VkDebugUtilsMessageSeverityFlagBitsEXT severity)
{
    switch (severity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: return cpplogger::Level::Error;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: return cpplogger::Level::Warn;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: return cpplogger::Level::Info;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: return cpplogger::Level::Debug;
        default: return cpplogger::Level::Trace;
    }
}

std::string VulkanMessageSeverity(const VkDebugUtilsMessageSeverityFlagBitsEXT severity)
{
    switch (severity) {
#define VKSWITCHCASE(X) \
    case X: return &#X[3];
        VKSWITCHCASE(VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        VKSWITCHCASE(VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        VKSWITCHCASE(VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        VKSWITCHCASE(VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
#undef VKSWITCHCASE
        default: return "";
    }
}

static const char *GetMessageSeverity(const VkDebugUtilsMessageSeverityFlagBitsEXT MsgSeverity)
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
    const char *Severity = GetMessageSeverity(MsgSeverity);

    LOG_V(LogVulkanRHI, VulkanMessageSeverityToLogLevel(MsgSeverity), "[{}:{}({})] {}", Severity,
          VulkanMessageType(messageType), pCallbackData->messageIdNumber, pCallbackData->pMessage);
    verify(false);
    return VK_FALSE;
}

void VulkanDynamicRHI::SetupDebugLayerCallback()
{
    PFN_vkCreateDebugUtilsMessengerEXT CreateDebugUtilsMessengerEXT =
        (PFN_vkCreateDebugUtilsMessengerEXT)(void *)VulkanAPI::vkGetInstanceProcAddr(m_Instance,
                                                                                     "vkCreateDebugUtilsMessengerEXT");
    if (CreateDebugUtilsMessengerEXT) {
        // vk::DebugUtilsMessengerCreateInfoEXT
        // {
        //     .messageSeverity =
        //         vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
        //         vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning,
        //     .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        //                    vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
        //                    vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        //     .pfnUserCallback = debugCallback,
        // }
        VkDebugUtilsMessengerCreateInfoEXT CreateInfo;
        ZeroVulkanStruct(CreateInfo, VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT);
        CreateInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        CreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        CreateInfo.pfnUserCallback = VulkanDebugUtilsMessengerCallback;
        VkResult Result = (*CreateDebugUtilsMessengerEXT)(m_Instance, &CreateInfo, nullptr, &Messenger);
        verify(Result == VK_SUCCESS);
    }
}

void VulkanDynamicRHI::RemoveDebugLayerCallback()
{
    if (Messenger != VK_NULL_HANDLE) {
        PFN_vkDestroyDebugUtilsMessengerEXT DestroyDebugUtilsMessengerEXT =
            (PFN_vkDestroyDebugUtilsMessengerEXT)(void *)VulkanAPI::vkGetInstanceProcAddr(
                m_Instance, "vkDestroyDebugUtilsMessengerEXT");
        if (DestroyDebugUtilsMessengerEXT) { (*DestroyDebugUtilsMessengerEXT)(m_Instance, Messenger, nullptr); }
    }
}

}    // namespace Raphael::RHI