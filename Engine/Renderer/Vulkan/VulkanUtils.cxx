#include "Engine/Renderer/Vulkan/VulkanUtils.hxx"

#include "Engine/Renderer/Vulkan/VulkanRHI.hxx"

#include <magic_enum.hpp>

namespace VulkanRHI
{

void VulkanCheckResult(VkResult Result, const char *VulkanFunction, const std::source_location &location)
{
    bool bDumpMemory = false;
    std::string_view ErrorString = magic_enum::enum_name(Result);

    switch (Result) {
        case VK_ERROR_OUT_OF_HOST_MEMORY:
        case VK_ERROR_OUT_OF_DEVICE_MEMORY: bDumpMemory = true; break;
        default: break;
    }

    // TODO: Do something with bDumpMemory
    (void)bDumpMemory;

    LOG(LogVulkanRHI, Fatal, "{} failed, VkResult={:s}\n\tat {}:{} with error {}", VulkanFunction,
        magic_enum::enum_name(Result), location.file_name(), location.line(), ErrorString);
    check(false);
}

}    // namespace VulkanRHI
