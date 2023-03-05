#pragma once

#include "Engine/Renderer/RHI/RHI.hxx"
#include "Engine/Renderer/RHI/RHIResource.hxx"

#include "Engine/Renderer/Vulkan/VulkanSwapChain.hxx"

// Include other VulkanResources
#include "Engine/Renderer/Vulkan/VulkanShader.hxx"
#include "Engine/Renderer/Vulkan/VulkanTexture.hxx"
#include "Engine/Renderer/Vulkan/VulkanViewport.hxx"

namespace VulkanRHI
{

class Semaphore;
class VulkanDevice;

class GraphicsPipelineCreationInfo
{
public:
    bool operator==(const GraphicsPipelineCreationInfo &) const = default;

public:
    EPrimitiveType PrimitiveType;
    EPixelFormat PixelFormat;
    RasterizerCreateInfo Rasterizer;
};

}    // namespace VulkanRHI
