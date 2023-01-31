#pragma once

#include <source_location>

#include "Engine/Renderer/Vulkan/VulkanLoader.hxx"

#define VK_CHECK_RESULT(f)                                                                       \
    {                                                                                            \
        const VkResult ScopedResult = (f);                                                       \
        if (ScopedResult != VK_SUCCESS) { ::Raphael::RHI::VulkanCheckResult(ScopedResult, #f); } \
    }

#define VK_CHECK_RESULT_EXPANDED(f)                                                             \
    {                                                                                           \
        const VkResult ScopedResult = (f);                                                      \
        if (ScopedResult < VK_SUCCESS) { ::Raphael::RHI::VulkanCheckResult(ScopedResult, #f); } \
    }

namespace Raphael::RHI
{

void VulkanCheckResult(VkResult result, const char *VulkanFunction,
                       const std::source_location &location = std::source_location::current());

template <class T>
requires(!std::is_pointer_v<T>)
FORCEINLINE void ZeroVulkanStruct(T &Struct, int32 StructureType)
{
    static_assert(offsetof(T, sType) == 0, "Assumes sType is the first member in the Vulkan type!");
    static_assert(sizeof(T::sType) == sizeof(int32), "Assumed sType is compatible with int32!");

    (int32 &)Struct.sType = StructureType;
    std::memset(((std::uint8_t *)&Struct) + sizeof(StructureType), 0, sizeof(T) - sizeof(StructureType));
}

template <typename BitsType>
constexpr bool VulkanHasAllFlags(VkFlags Flags, BitsType Contains)
{
    return (Flags & Contains) == Contains;
}

}    // namespace Raphael::RHI
