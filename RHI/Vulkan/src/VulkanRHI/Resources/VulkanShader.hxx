#pragma once

#include "Engine/Core/RHI/Resources/RHIShader.hxx"

#include "VulkanRHI/VulkanLoader.hxx"

namespace VulkanRHI
{

class VulkanDevice;

class VulkanShader : public RHIShader
{
public:
    class ShaderHandle : public RObject
    {
    public:
        ShaderHandle() = delete;
        ShaderHandle(Ref<VulkanDevice>& InDevice, const VkShaderModuleCreateInfo& Info);
        ~ShaderHandle();

        VkShaderModule Handle;

    private:
        Ref<VulkanDevice> Device;
    };

public:
    VulkanShader(RHIShaderType Type, Array<uint32> InSPRIVCode);

    Ref<ShaderHandle> GetHandle(Ref<VulkanDevice> InDevice);

private:
    Array<uint32> SPIRVCode;
    WeakRef<ShaderHandle> m_ShaderHandle;

    friend class VulkanShaderCompiler;
};

}    // namespace VulkanRHI
