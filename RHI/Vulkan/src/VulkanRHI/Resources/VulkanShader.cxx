#include "VulkanRHI/Resources/VulkanShader.hxx"

#include "VulkanRHI/VulkanDevice.hxx"
#include "VulkanRHI/VulkanUtils.hxx"

#include "Engine/Core/RHI/RHIDefinitions.hxx"

namespace VulkanRHI
{

// Serialization

void ShaderResource::FStageIO::Serialize(Serialization::FStreamWriter* Writer, const ShaderResource::FStageIO& Value)
{
    Writer->WriteString(Value.Name);
    Writer->WriteRaw(Value.Type);
    Writer->WriteRaw(Value.Binding);
    Writer->WriteRaw(Value.Location);
}

void ShaderResource::FPushConstantRange::Serialize(Serialization::FStreamWriter* Writer,
                                                   const ShaderResource::FPushConstantRange& Value)
{
    Writer->WriteRaw(Value.Offset);
    Writer->WriteRaw(Value.Size);
    Writer->WriteObject(Value.Parameter);
}

void ShaderResource::FStorageBuffer::Serialize(Serialization::FStreamWriter* Writer,
                                               const ShaderResource::FStorageBuffer& Value)
{
    Writer->WriteRaw(Value.Set);
    Writer->WriteRaw(Value.Binding);
    Writer->WriteObject(Value.Parameter);
}

void ShaderResource::FUniformBuffer::Serialize(Serialization::FStreamWriter* Writer, const FUniformBuffer& Value)
{
    Writer->WriteRaw(Value.Set);
    Writer->WriteRaw(Value.Binding);
    Writer->WriteObject(Value.Parameter);
}

void RVulkanShader::FReflectionData::Serialize(Serialization::FStreamWriter* Writer, const FReflectionData& Value)
{
    Writer->WriteArray<ShaderResource::FStageIO>(Value.StageInput);
    Writer->WriteArray<ShaderResource::FStageIO>(Value.StageOutput);
    Writer->WriteObject<ShaderResource::FPushConstantRange>(Value.PushConstants);
    Writer->WriteArray<ShaderResource::FStorageBuffer>(Value.StorageBuffers);
}

// Deserialization

void ShaderResource::FStageIO::Deserialize(Serialization::FStreamReader* Reader, ShaderResource::FStageIO& OutValue)
{
    Reader->ReadString(OutValue.Name);
    Reader->ReadRaw(OutValue.Type);
    Reader->ReadRaw(OutValue.Binding);
    Reader->ReadRaw(OutValue.Location);
}

void ShaderResource::FPushConstantRange::Deserialize(Serialization::FStreamReader* Reader,
                                                     ShaderResource::FPushConstantRange& OutValue)
{
    Reader->ReadRaw(OutValue.Offset);
    Reader->ReadRaw(OutValue.Size);
    Reader->ReadObject(OutValue.Parameter);
}

void ShaderResource::FStorageBuffer::Deserialize(Serialization::FStreamReader* Reader,
                                                 ShaderResource::FStorageBuffer& OutValue)
{
    Reader->ReadRaw(OutValue.Set);
    Reader->ReadRaw(OutValue.Binding);
    Reader->ReadObject(OutValue.Parameter);
}

void ShaderResource::FUniformBuffer::Deserialize(Serialization::FStreamReader* Reader, FUniformBuffer& OutValue)
{
    Reader->ReadRaw(OutValue.Set);
    Reader->ReadRaw(OutValue.Binding);
    Reader->ReadObject(OutValue.Parameter);
}

void RVulkanShader::FReflectionData::Deserialize(Serialization::FStreamReader* Reader, FReflectionData& OutValue)
{
    Reader->ReadArray<ShaderResource::FStageIO>(OutValue.StageInput);
    Reader->ReadArray<ShaderResource::FStageIO>(OutValue.StageOutput);
    Reader->ReadObject<ShaderResource::FPushConstantRange>(OutValue.PushConstants);
    Reader->ReadArray<ShaderResource::FStorageBuffer>(OutValue.StorageBuffers);
}

RVulkanShader::RVulkanShaderHandle::RVulkanShaderHandle(FVulkanDevice* InDevice, const VkShaderModuleCreateInfo& Info)
    : IDeviceChild(InDevice)
{
    VK_CHECK_RESULT(VulkanAPI::vkCreateShaderModule(Device->GetHandle(), &Info, VULKAN_CPU_ALLOCATOR, &Handle));
}

RVulkanShader::RVulkanShaderHandle::~RVulkanShaderHandle()
{
    RHI::DeferedDeletion([Handle = this->Handle, Device = this->Device] {
        VulkanAPI::vkDestroyShaderModule(Device->GetHandle(), Handle, VULKAN_CPU_ALLOCATOR);
    });
}

void RVulkanShader::RVulkanShaderHandle::SetName(std::string_view Name)
{
    Super::SetName(Name);
    VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_SHADER_MODULE, Handle, "{:s}", Name);
}

RVulkanShader::RVulkanShader(ERHIShaderType Type, const TArray<uint32>& InSPIRVCode,
                             const FReflectionData& InReflectionData)
    : Super(Type), SPIRVCode(InSPIRVCode), m_ReflectionData(InReflectionData), Type(Type)
{
    ShaderModuleCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .codeSize = SPIRVCode.Size() * sizeof(uint32),
        .pCode = SPIRVCode.Raw(),
    };
    CreateDescriptorSetLayout();
}

RVulkanShader::~RVulkanShader()
{
    for (VkDescriptorSetLayout& Layout: DescriptorSetLayouts) {
        VulkanAPI::vkDestroyDescriptorSetLayout(GetVulkanDynamicRHI()->GetDevice()->GetHandle(), Layout,
                                                VULKAN_CPU_ALLOCATOR);
    }
}

const VkShaderModuleCreateInfo& RVulkanShader::GetShaderModuleCreateInfo() const
{
    return ShaderModuleCreateInfo;
}

const char* RVulkanShader::GetEntryPoint() const
{
    return "main";
}

void RVulkanShader::CreateDescriptorSetLayout()
{
    TArray<TArray<VkDescriptorSetLayoutBinding>> DescriptorSetBindings;
    // Not the most efficient, but this will do for now
    for (const ShaderResource::FStorageBuffer& Buffer: m_ReflectionData.StorageBuffers) {
        if (Buffer.Set >= DescriptorSetBindings.Size()) {
            DescriptorSetBindings.Resize(Buffer.Set + 1);
        }
        VkDescriptorSetLayoutBinding* FoundBinding = DescriptorSetBindings[Buffer.Set].FindByLambda(
            [Buffer](const VkDescriptorSetLayoutBinding& Binding) { return Binding.binding == Buffer.Binding; });
        if (FoundBinding) {
            FoundBinding->stageFlags |= ConvertToVulkanType(GetShaderType());
            continue;
        }

        VkDescriptorSetLayoutBinding Binding{
            .binding = Buffer.Binding,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1,
            .stageFlags = static_cast<VkShaderStageFlags>(ConvertToVulkanType(GetShaderType())),
            .pImmutableSamplers = nullptr,
        };
        DescriptorSetBindings[Buffer.Set].Add(Binding);
    }
    if (m_ReflectionData.StorageBuffers.Size() > 0) {
        DescriptorPoolSizes.Add(
            {.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .descriptorCount = m_ReflectionData.StorageBuffers.Size()});
    }

    for (const ShaderResource::FUniformBuffer& Buffer: m_ReflectionData.UniformBuffers) {
        if (Buffer.Set >= DescriptorSetBindings.Size()) {
            DescriptorSetBindings.Resize(Buffer.Set + 1);
        }
        VkDescriptorSetLayoutBinding* FoundBinding = DescriptorSetBindings[Buffer.Set].FindByLambda(
            [Buffer](const VkDescriptorSetLayoutBinding& Binding) { return Binding.binding == Buffer.Binding; });
        if (FoundBinding) {
            FoundBinding->stageFlags |= ConvertToVulkanType(GetShaderType());
            continue;
        }

        VkDescriptorSetLayoutBinding Binding{
            .binding = Buffer.Binding,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = static_cast<VkShaderStageFlags>(ConvertToVulkanType(GetShaderType())),
            .pImmutableSamplers = nullptr,
        };
        DescriptorSetBindings[Buffer.Set].Add(Binding);
    }
    if (m_ReflectionData.UniformBuffers.Size() > 0) {
        DescriptorPoolSizes.Add(
            {.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = m_ReflectionData.UniformBuffers.Size()});
    }

    DescriptorSetLayouts.Resize(DescriptorSetBindings.Size());
    for (unsigned Set = 0; Set < DescriptorSetBindings.Size(); Set++) {
        const VkDescriptorSetLayoutCreateInfo CreateInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .bindingCount = DescriptorSetBindings[Set].Size(),
            .pBindings = DescriptorSetBindings[Set].Raw(),
        };
        VK_CHECK_RESULT(VulkanAPI::vkCreateDescriptorSetLayout(GetVulkanDynamicRHI()->GetDevice()->GetHandle(),
                                                               &CreateInfo, VULKAN_CPU_ALLOCATOR,
                                                               &DescriptorSetLayouts[Set]));
    }
}

}    // namespace VulkanRHI
