#include "Engine/Renderer/Vulkan/VulkanShader.hxx"

#include "Engine/Renderer/Vulkan/VulkanDevice.hxx"

namespace VulkanRHI
{

VulkanShader::VulkanShader(RHIShaderType Type, std::vector<uint32> InSPIRVCode)
    : RHIShader(Type), SPIRVCode(InSPIRVCode), ShaderModule(VK_NULL_HANDLE)
{
}

uint32 VulkanShader::GetID()
{
    return 0;
}

VkShaderModule VulkanShader::GetHandle(Ref<VulkanDevice> InDevice)
{
    if (ShaderModule == VK_NULL_HANDLE) {
        VkShaderModuleCreateInfo CreateInfo{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = SPIRVCode.size() * sizeof(uint32),
            .pCode = SPIRVCode.data(),
        };
        VulkanAPI::vkCreateShaderModule(InDevice->GetInstanceHandle(), &CreateInfo, nullptr, &ShaderModule);
    }
    return ShaderModule;
}

}    // namespace VulkanRHI
