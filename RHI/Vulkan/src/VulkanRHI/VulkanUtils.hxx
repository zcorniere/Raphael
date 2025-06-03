#pragma once

#include <source_location>

#include "Engine/Core/RHI/RHIDefinitions.hxx"
#include "Engine/Core/RHI/Resources/RHIShader.hxx"

#define VK_CHECK_RESULT(f)                                  \
    {                                                       \
        const VkResult ScopedResult = (f);                  \
        if (ScopedResult != VK_SUCCESS)                     \
        {                                                   \
            VulkanRHI::VulkanCheckResult(ScopedResult, #f); \
        }                                                   \
    }

#define VK_CHECK_RESULT_EXPANDED(f)                         \
    {                                                       \
        const VkResult ScopedResult = (f);                  \
        if (ScopedResult < VK_SUCCESS)                      \
        {                                                   \
            VulkanRHI::VulkanCheckResult(ScopedResult, #f); \
        }                                                   \
    }

#include <vulkan/vk_enum_string_helper.h>
#ifndef NDEBUG
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
    switch (Type)
    {
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

/// Transform RHI Image Format to VK_FORMAT_*
FORCEINLINE VkFormat ImageFormatToFormat(const EImageFormat Format)
{
    switch (Format)
    {
        case EImageFormat::D32_SFLOAT:
            return VK_FORMAT_D32_SFLOAT;
        case EImageFormat::R8G8B8_SRGB:
            return VK_FORMAT_R8G8B8_SRGB;
        case EImageFormat::R8G8B8A8_SRGB:
            return VK_FORMAT_R8G8B8A8_SRGB;
        case EImageFormat::B8G8R8A8_SRGB:
            return VK_FORMAT_B8G8R8A8_SRGB;
    }
    checkNoEntry();
}
/// Transform RHI Image Format to VK_FORMAT_*
FORCEINLINE EImageFormat VkFormatToImageFormat(const VkFormat Format)
{
    switch (Format)
    {
        case VK_FORMAT_D32_SFLOAT:
            return EImageFormat::D32_SFLOAT;
        case VK_FORMAT_R8G8B8_SRGB:
            return EImageFormat::R8G8B8_SRGB;
        case VK_FORMAT_R8G8B8A8_SRGB:
            return EImageFormat::R8G8B8A8_SRGB;
        case VK_FORMAT_B8G8R8A8_SRGB:
            return EImageFormat::B8G8R8A8_SRGB;
        default:
            break;
    }
    checkNoEntry();
}

FORCEINLINE VkImageViewType TextureDimensionToVkImageViewType(EImageDimension Dimension)
{
    switch (Dimension)
    {
        case EImageDimension::Texture2D:
            return VK_IMAGE_VIEW_TYPE_2D;
    }
    checkNoEntry();
}

FORCEINLINE VkImageAspectFlags TextureUsageFlagToVkImageAspectFlags(ETextureUsageFlags CreateFlags)
{
    VkImageAspectFlags ReturnFlag = VK_IMAGE_ASPECT_NONE;

    if (EnumHasAnyFlags(CreateFlags, ETextureUsageFlags::RenderTargetable | ETextureUsageFlags::ResolveTargetable))
    {
        ReturnFlag |= VK_IMAGE_ASPECT_COLOR_BIT;
    }
    if (EnumHasAnyFlags(CreateFlags, ETextureUsageFlags::DepthStencilTargetable))
    {
        ReturnFlag |= VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    return ReturnFlag;
}

FORCEINLINE VkAttachmentLoadOp RenderTargetLoadActionToVkAttachmentLoadOp(ERenderTargetLoadAction Action)
{
    switch (Action)
    {
        case ERenderTargetLoadAction::Load:
            return VK_ATTACHMENT_LOAD_OP_LOAD;
        case ERenderTargetLoadAction::Clear:
            return VK_ATTACHMENT_LOAD_OP_CLEAR;
        case ERenderTargetLoadAction::NoAction:
            return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    }
    checkNoEntry();
}

FORCEINLINE VkAttachmentStoreOp RenderTargetStoreActionToVkAttachmentStoreOp(ERenderTargetStoreAction Action)
{
    switch (Action)
    {
        case ERenderTargetStoreAction::Store:
            return VK_ATTACHMENT_STORE_OP_STORE;
        case ERenderTargetStoreAction::NoAction:
            return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    }
    checkNoEntry();
}

FORCEINLINE VkImageUsageFlags TextureUsageFlagsToVkImageUsageFlags(ETextureUsageFlags CreateFlags)
{
    VkImageUsageFlags ReturnFlag = 0;
    if (EnumHasAnyFlags(CreateFlags, ETextureUsageFlags::SampleTargetable))
    {
        ReturnFlag |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    if (EnumHasAnyFlags(CreateFlags, ETextureUsageFlags::TransferTargetable))
    {
        ReturnFlag |= VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    if (EnumHasAnyFlags(CreateFlags, ETextureUsageFlags::RenderTargetable | ETextureUsageFlags::ResolveTargetable))
    {
        ReturnFlag |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    if (EnumHasAnyFlags(CreateFlags, ETextureUsageFlags::DepthStencilTargetable))
    {
        ReturnFlag |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    return ReturnFlag;
}

FORCEINLINE VkImageType TextureDimensionToVkImageType(EImageDimension Dimension)
{
    switch (Dimension)
    {
        case EImageDimension::Texture2D:
            return VK_IMAGE_TYPE_2D;
    }
    checkNoEntry();
}

FORCEINLINE VkVertexInputRate ConvertToVulkanType(EVertexInputMode Mode)
{
    switch (Mode)
    {
        case EVertexInputMode::PerVertex:
            return VK_VERTEX_INPUT_RATE_VERTEX;
        case EVertexInputMode::PerInstance:
            return VK_VERTEX_INPUT_RATE_INSTANCE;
    }
    checkNoEntry();
}

/// Convert the PolygonMode to VK_POLYGON_*
FORCEINLINE VkPolygonMode ConvertToVulkanType(EPolygonMode Mode)
{
    switch (Mode)
    {
        case EPolygonMode::Fill:
            return VK_POLYGON_MODE_FILL;
        case EPolygonMode::Line:
            return VK_POLYGON_MODE_LINE;
        case EPolygonMode::Point:
            return VK_POLYGON_MODE_POINT;
    }
    checkNoEntry();
}

/// Convert the ShaderType to VK_SHADER_STAGE*
FORCEINLINE VkShaderStageFlagBits ConvertToVulkanType(ERHIShaderType Type)
{
    switch (Type)
    {
        case ERHIShaderType::Vertex:
            return VK_SHADER_STAGE_VERTEX_BIT;
        case ERHIShaderType::Fragment:
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        case ERHIShaderType::Compute:
            return VK_SHADER_STAGE_COMPUTE_BIT;
    }
    checkNoEntry();
}

/// Convert the Cull mode to VK_CULL_*
FORCEINLINE VkCullModeFlags ConvertToVulkanType(ECullMode Mode)
{
    switch (Mode)
    {
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
    switch (Mode)
    {
        case EFrontFace::Clockwise:
            return VK_FRONT_FACE_CLOCKWISE;
        case EFrontFace::CounterClockwise:
            return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    }
    checkNoEntry();
}

}    // namespace VulkanRHI
