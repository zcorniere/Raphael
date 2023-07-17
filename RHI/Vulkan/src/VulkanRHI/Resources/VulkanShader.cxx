#include "VulkanRHI/Resources/VulkanShader.hxx"

#include "VulkanRHI/VulkanDevice.hxx"
#include "VulkanShader.hxx"

namespace VulkanRHI
{

VulkanShader::ShaderHandle::ShaderHandle(VulkanDevice* InDevice, const VkShaderModuleCreateInfo& Info): Device(InDevice)
{
    VK_CHECK_RESULT(VulkanAPI::vkCreateShaderModule(Device->GetHandle(), &Info, nullptr, &Handle));
}

VulkanShader::ShaderHandle::~ShaderHandle()
{
    VulkanAPI::vkDestroyShaderModule(Device->GetHandle(), Handle, nullptr);
}

VulkanShader::VulkanShader(RHIShaderType Type, const Array<uint32>& InSPIRVCode, const ReflectionData& InReflectionData)
    : RHIShader(Type), SPIRVCode(InSPIRVCode), m_ReflectionData(InReflectionData), Type(Type), m_ShaderHandle(nullptr)
{
}

Ref<VulkanShader::ShaderHandle> VulkanShader::GetHandle(VulkanDevice* InDevice)
{
    if (m_ShaderHandle) {
        return Ref(m_ShaderHandle);
    }
    VkShaderModuleCreateInfo CreateInfo{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = SPIRVCode.Size() * sizeof(uint32),
        .pCode = SPIRVCode.Raw(),
    };
    m_ShaderHandle = Ref<ShaderHandle>::Create(InDevice, CreateInfo);
    return m_ShaderHandle;
}

}    // namespace VulkanRHI
