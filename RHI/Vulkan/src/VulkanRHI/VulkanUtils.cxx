#include "VulkanRHI/VulkanUtils.hxx"

#include "VulkanRHI/VulkanDevice.hxx"
#include "VulkanRHI/VulkanMemoryManager.hxx"
#include "VulkanRHI/VulkanRHI.hxx"

#include <magic_enum.hpp>

namespace VulkanRHI
{

void VulkanCheckResult(VkResult Result, const char* VulkanFunction, const std::source_location& location)
{
    bool bDumpMemory = false;
    std::string_view ErrorString = magic_enum::enum_name(Result);

    switch (Result) {
        case VK_ERROR_OUT_OF_HOST_MEMORY:
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            bDumpMemory = true;
            break;
        default:
            break;
    }

    if (bDumpMemory) {
        char* String = nullptr;
        vmaBuildStatsString(GetVulkanDynamicRHI()->GetDevice()->GetMemoryManager()->GetAllocator(), &String, VK_TRUE);
        LOG(LogVulkanRHI, Fatal, "VMA DUMP : \n{}", String);
        vmaFreeStatsString(GetVulkanDynamicRHI()->GetDevice()->GetMemoryManager()->GetAllocator(), String);
    }

    LOG(LogVulkanRHI, Fatal, "{} failed, VkResult={:s}\n\tat {}:{} with error {}", VulkanFunction,
        magic_enum::enum_name(Result), location.file_name(), location.line(), ErrorString);
    check(false);
}

}    // namespace VulkanRHI
