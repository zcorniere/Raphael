#pragma once

#include <source_location>

#include "Engine/Core/RHI/RHIDefinitions.hxx"
#include "VulkanRHI/VulkanLoader.hxx"

#define VK_CHECK_RESULT(f)                                  \
    {                                                       \
        const VkResult ScopedResult = (f);                  \
        if (ScopedResult != VK_SUCCESS) {                   \
            VulkanRHI::VulkanCheckResult(ScopedResult, #f); \
        }                                                   \
    }

#define VK_CHECK_RESULT_EXPANDED(f)                         \
    {                                                       \
        const VkResult ScopedResult = (f);                  \
        if (ScopedResult < VK_SUCCESS) {                    \
            VulkanRHI::VulkanCheckResult(ScopedResult, #f); \
        }                                                   \
    }

#ifndef NDEBUG
    #include <vk_enum_string_helper.h>
    #define VK_TYPE_TO_STRING(Type, Value) string_##Type(Value)
#else
    #define VK_TYPE_TO_STRING(Type, Value) magic_enum::enum_name(Value)
#endif

namespace VulkanRHI
{

void VulkanCheckResult(VkResult result, const char* VulkanFunction,
                       const std::source_location& location = std::source_location::current());

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

/// Convert the RHI VertexElementType to VK_FORMAT_*
FORCEINLINE VkFormat VertexElementToFormat(const EVertexElementType Type)
{
    switch (Type) {
        case EVertexElementType::Float1:
            return VK_FORMAT_R32_SFLOAT;
        case EVertexElementType::Float2:
            return VK_FORMAT_R32G32_SFLOAT;
        case EVertexElementType::Float3:
            return VK_FORMAT_R32G32B32_SFLOAT;
        case EVertexElementType::Float4:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        case EVertexElementType::Uint1:
            return VK_FORMAT_R32_UINT;
        case EVertexElementType::Uint2:
            return VK_FORMAT_R32G32_UINT;
        case EVertexElementType::Uint3:
            return VK_FORMAT_R32G32B32_UINT;
        case EVertexElementType::Uint4:
            return VK_FORMAT_R32G32B32A32_UINT;
        case EVertexElementType::Int1:
            return VK_FORMAT_R32_SINT;
        case EVertexElementType::Int2:
            return VK_FORMAT_R32G32_SINT;
        case EVertexElementType::Int3:
            return VK_FORMAT_R32G32B32_SINT;
        case EVertexElementType::Int4:
            return VK_FORMAT_R32G32B32A32_SINT;
    }
    checkNoEntry();
}

/// Transform RHI Image Format to VC_FORMAT_*
FORCEINLINE VkFormat ImageFormatToFormat(const EImageFormat Format)
{
    switch (Format) {
        case EImageFormat::R8G8B8_SRGB:
            return VK_FORMAT_R8G8B8_SRGB;
        case EImageFormat::R8G8B8A8_SRGB:
            return VK_FORMAT_R8G8B8A8_SRGB;
    }
    checkNoEntry();
}

/// Convert the PolygonMode to VK_POLYGON_*
FORCEINLINE VkPolygonMode ConvertToVulkanType(EPolygonMode Mode)
{
    switch (Mode) {
        case EPolygonMode::Fill:
            return VK_POLYGON_MODE_FILL;
        case EPolygonMode::Line:
            return VK_POLYGON_MODE_LINE;
        case EPolygonMode::Point:
            return VK_POLYGON_MODE_POINT;
    }
    checkNoEntry();
}

/// Convert the Cull mode to VK_CULL_*
FORCEINLINE VkCullModeFlags ConvertToVulkanType(ECullMode Mode)
{
    switch (Mode) {
        case ECullMode::None:
            return VK_CULL_MODE_NONE;
        case ECullMode::Back:
            return VK_CULL_MODE_BACK_BIT;
        case ECullMode::Front:
            return VK_CULL_MODE_FRONT_BIT;
        case ECullMode::FrontAndBack:
            return VK_CULL_MODE_FRONT_AND_BACK;
    }
    checkNoEntry();
}

/// Convert the Front face to Vulkan  VK_FRONT_FACE_*
FORCEINLINE VkFrontFace ConvertToVulkanType(EFrontFace Mode)
{
    switch (Mode) {
        case EFrontFace::Clockwise:
            return VK_FRONT_FACE_CLOCKWISE;
        case EFrontFace::CounterClockwise:
            return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    }
    checkNoEntry();
}

}    // namespace VulkanRHI
