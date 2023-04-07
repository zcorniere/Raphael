#pragma once

#include <source_location>

#include "Engine/Renderer/Vulkan/VulkanLoader.hxx"

#define VK_CHECK_RESULT(f)                                                         \
    {                                                                              \
        const VkResult ScopedResult = (f);                                         \
        if (ScopedResult != VK_SUCCESS) { VulkanRHI::VulkanCheckResult(ScopedResult, #f); } \
    }

#define VK_CHECK_RESULT_EXPANDED(f)                                               \
    {                                                                             \
        const VkResult ScopedResult = (f);                                        \
        if (ScopedResult < VK_SUCCESS) { VulkanRHI::VulkanCheckResult(ScopedResult, #f); } \
    }

#ifndef NDEBUG
    #include "vk_enum_string_helper.h"
    #define VK_TYPE_TO_STRING(Type, Value) string_##Type(Value)
#else
    #define VK_TYPE_TO_STRING(Type, Value) Value
#endif

namespace VulkanRHI
{

void VulkanCheckResult(VkResult result, const char *VulkanFunction,
                       const std::source_location &location = std::source_location::current());

template <typename BitsType>
constexpr bool VulkanHasAllFlags(VkFlags Flags, BitsType Contains)
{
    return (Flags & Contains) == Contains;
}

template <typename BitsType>
constexpr bool VKHasAnyFlags(VkFlags Flags, BitsType Contains)
{
    return (Flags & Contains) != 0;
}

}    // namespace VulkanRHI
