#include "VulkanRHI/VulkanRHI_Debug.hxx"

#include "VulkanRHI/VulkanRHI.hxx"

#include "VulkanRHI/VulkanLoader.hxx"
#include "VulkanRHI/VulkanUtils.hxx"

#include <ranges>

#if VULKAN_DEBUGGING_ENABLED

static std::string_view VulkanMessageType(const VkDebugUtilsMessageTypeFlagsEXT& s)
{
    switch (s) {
        case 7:
            return "General | Validation | Performance";
        case 6:
            return "Validation | Performance";
        case 5:
            return "General | Performance";
        case 4 /*VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT*/:
            return "Performance";
        case 3:
            return "General | Validation";
        case 2 /*VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT*/:
            return "Validation";
        case 1 /*VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT*/:
            return "General";
        default:
            return "Unknown";
    }
}

static cpplogger::Level VulkanMessageSeverityToLogLevel(const VkDebugUtilsMessageSeverityFlagBitsEXT severity)
{
    switch (severity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            return cpplogger::Level::Error;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            return cpplogger::Level::Warning;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            return cpplogger::Level::Info;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            return cpplogger::Level::Info;
        default:
            return cpplogger::Level::Trace;
    }
}

static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugUtilsMessengerCallback(
    const VkDebugUtilsMessageSeverityFlagBitsEXT MsgSeverity, const VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void*)
{
    std::string Objects;
    if (pCallbackData->objectCount) {
        Objects = std::format("\tObjects({}): \n", pCallbackData->objectCount);
        for (uint32_t i = 0; i < pCallbackData->objectCount; ++i) {
            const auto& object = pCallbackData->pObjects[i];
            Objects.append(std::format("\t\t- Object[{0}] name: {1}, type: {2}, handle: {3:#x}\n", i,
                                       object.pObjectName ? object.pObjectName : "NULL",
                                       VK_TYPE_TO_STRING(VkObjectType, object.objectType), object.objectHandle));
        }
    }

    std::string InterestingPart(pCallbackData->pMessage);
    std::size_t Size = InterestingPart.find_last_of('|');
    if (Size == InterestingPart.npos) {
        Size = 0;
    } else {
        Size += 2;
    }

    LOG_V(LogVulkanRHI, VulkanMessageSeverityToLogLevel(MsgSeverity), "{:s} [ {:s} ]\n\t{:s}\n{:s}",
          VulkanMessageType(messageType), pCallbackData->pMessageIdName, InterestingPart.data() + Size, Objects);

    if (FPlatform::isDebuggerPresent()) {
        PLATFORM_BREAK();
    }
    return VK_FALSE;
}

namespace VulkanRHI
{

TArray<const char*> VulkanRHI_Debug::GetSupportedInstanceLayers()
{
    static const TArray<const char*> ExpectedValidationLayers{"VK_LAYER_KHRONOS_validation"};
    TArray<const char*> FoundLayers;

    uint32 PropertiesCount;
    TArray<VkLayerProperties> AvailableLayers;
    VulkanAPI::vkEnumerateInstanceLayerProperties(&PropertiesCount, nullptr);
    AvailableLayers.Resize(PropertiesCount);
    VulkanAPI::vkEnumerateInstanceLayerProperties(&PropertiesCount, AvailableLayers.Raw());

    for (const VkLayerProperties& Properties: AvailableLayers) {
        for (const char* ExpectedLayer: ExpectedValidationLayers) {
            if (std::strcmp(ExpectedLayer, Properties.layerName) == 0) {
                FoundLayers.Add(ExpectedLayer);
            }
        }
    }
    if (FoundLayers.Size() != ExpectedValidationLayers.Size()) {
        bValidationLayersAreMissing = true;
        auto FilterLambda = [&FoundLayers](const char* LayerName) {
            for (const char* Layer: FoundLayers) {
                if (std::strcmp(Layer, LayerName) == 0)
                    return false;
            }
            return true;
        };
        TArray<const char*> MissingLayer;
        LOG(LogVulkanRHI, Error, "Some Validation layers were not found !");
        for (const char* Layer: ExpectedValidationLayers | std::views::filter(FilterLambda)) {
            LOG(LogVulkanRHI, Error, "- {}", Layer);
        }
    }
    return FoundLayers;
}

void VulkanRHI_Debug::SetupDebugLayer(VkInstance Instance)
{
    VkDebugUtilsMessengerCreateInfoEXT CreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = VulkanDebugUtilsMessengerCallback,
    };
    VkResult Result =
        VulkanAPI::vkCreateDebugUtilsMessengerEXT(Instance, &CreateInfo, VULKAN_CPU_ALLOCATOR, &Messenger);
    ensure(Result == VK_SUCCESS);
}

void VulkanRHI_Debug::RemoveDebugLayer(VkInstance Instance)
{
    if (Messenger != VK_NULL_HANDLE) {
        VulkanAPI::vkDestroyDebugUtilsMessengerEXT(Instance, Messenger, VULKAN_CPU_ALLOCATOR);
    }
}

}    // namespace VulkanRHI

#endif    // VULKAN_DEBUGGING_ENABLED
