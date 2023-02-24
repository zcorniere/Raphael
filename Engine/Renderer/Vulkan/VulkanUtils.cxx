#include "Engine/Renderer/Vulkan/VulkanUtils.hxx"

#include "Engine/Renderer/Vulkan/IVulkanDynamicRHI.hxx"

static std::string GetErrorString(VkResult Result)
{
    switch (Result) {
#define VKERRORCASE(x) \
    case x: return #x
        VKERRORCASE(VK_NOT_READY);
        VKERRORCASE(VK_TIMEOUT);
        VKERRORCASE(VK_EVENT_SET);
        VKERRORCASE(VK_EVENT_RESET);
        VKERRORCASE(VK_INCOMPLETE);
        VKERRORCASE(VK_ERROR_OUT_OF_HOST_MEMORY);
        VKERRORCASE(VK_ERROR_OUT_OF_DEVICE_MEMORY);
        VKERRORCASE(VK_ERROR_INITIALIZATION_FAILED);
        VKERRORCASE(VK_ERROR_DEVICE_LOST);
        VKERRORCASE(VK_ERROR_MEMORY_MAP_FAILED);
        VKERRORCASE(VK_ERROR_LAYER_NOT_PRESENT);
        VKERRORCASE(VK_ERROR_EXTENSION_NOT_PRESENT);
        VKERRORCASE(VK_ERROR_FEATURE_NOT_PRESENT);
        VKERRORCASE(VK_ERROR_INCOMPATIBLE_DRIVER);
        VKERRORCASE(VK_ERROR_TOO_MANY_OBJECTS);
        VKERRORCASE(VK_ERROR_FORMAT_NOT_SUPPORTED);
        VKERRORCASE(VK_ERROR_SURFACE_LOST_KHR);
        VKERRORCASE(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR);
        VKERRORCASE(VK_SUBOPTIMAL_KHR);
        VKERRORCASE(VK_ERROR_OUT_OF_DATE_KHR);
        VKERRORCASE(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR);
        VKERRORCASE(VK_ERROR_VALIDATION_FAILED_EXT);
#if VK_HEADER_VERSION >= 13
        VKERRORCASE(VK_ERROR_INVALID_SHADER_NV);
#endif
#if VK_HEADER_VERSION >= 24
        VKERRORCASE(VK_ERROR_FRAGMENTED_POOL);
#endif
#if VK_HEADER_VERSION >= 39
        VKERRORCASE(VK_ERROR_OUT_OF_POOL_MEMORY_KHR);
#endif
#if VK_HEADER_VERSION >= 65
        VKERRORCASE(VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR);
        VKERRORCASE(VK_ERROR_NOT_PERMITTED_EXT);
#endif
#undef VKERRORCASE
        default: return "";
    }
}

namespace VulkanRHI
{

void VulkanCheckResult(VkResult Result, const char *VulkanFunction, const std::source_location &location)
{
    bool bDumpMemory = false;
    std::string ErrorString = GetErrorString(Result);

    switch (Result) {
        case VK_ERROR_OUT_OF_HOST_MEMORY:
        case VK_ERROR_OUT_OF_DEVICE_MEMORY: bDumpMemory = true; break;
        default: break;
    }

    // TODO: Do something with bDumpMemory
    (void)bDumpMemory;

    LOG(LogVulkanRHI, Error, "{} failed, VkResult={:d}\n at {}:{} with error {}", VulkanFunction, Result,
        location.file_name(), location.line(), ErrorString);
    verifyAlways(false);
}

}    // namespace VulkanRHI
