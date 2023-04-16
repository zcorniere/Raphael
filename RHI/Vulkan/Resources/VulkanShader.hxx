#pragma once

#include "Engine/Core/RHI/Resources/RHIShader.hxx"

#include "RHI/Vulkan/VulkanLoader.hxx"

namespace VulkanRHI
{

class VulkanDevice;

class VulkanShader : public RHIShader
{
public:
    VulkanShader(RHIShaderType Type, std::vector<uint32> InSPRIVCode);

    uint32 GetID();
    VkShaderModule GetHandle(Ref<VulkanDevice> InDevice);

private:
    std::vector<uint32> SPIRVCode;
    VkShaderModule ShaderModule;

    friend class VulkanShaderCompiler;
};

}    // namespace VulkanRHI
