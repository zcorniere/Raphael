#include "RHI/Vulkan/Resources/VulkanShader.hxx"

#include "RHI/Vulkan/VulkanDevice.hxx"

namespace VulkanRHI
{

VulkanShader::ShaderHandle::ShaderHandle(Ref<VulkanDevice> &InDevice, const VkShaderModuleCreateInfo &Info)
    : Device(InDevice)
{
    VK_CHECK_RESULT(VulkanAPI::vkCreateShaderModule(Device->GetInstanceHandle(), &Info, nullptr, &Handle));
}

VulkanShader::ShaderHandle::~ShaderHandle()
{
    VulkanAPI::vkDestroyShaderModule(Device->GetInstanceHandle(), Handle, nullptr);
}

VulkanShader::VulkanShader(RHIShaderType Type, std::vector<uint32> InSPIRVCode)
    : RHIShader(Type), SPIRVCode(InSPIRVCode), m_ShaderHandle(nullptr)
{
}

Ref<VulkanShader::ShaderHandle> VulkanShader::GetHandle(Ref<VulkanDevice> InDevice)
{
    if (m_ShaderHandle.IsValid()) { return Ref(m_ShaderHandle); }
    VkShaderModuleCreateInfo CreateInfo{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = SPIRVCode.size() * sizeof(uint32),
        .pCode = SPIRVCode.data(),
    };
    m_ShaderHandle = Ref<ShaderHandle>::Create(InDevice, CreateInfo);
    return Ref(m_ShaderHandle);
}

}    // namespace VulkanRHI
