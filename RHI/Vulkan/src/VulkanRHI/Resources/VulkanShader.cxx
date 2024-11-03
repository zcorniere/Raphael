#include "VulkanRHI/Resources/VulkanShader.hxx"

#include "Engine/Core/RHI/RHIDefinitions.hxx"
#include "VulkanRHI/VulkanDevice.hxx"
#include "VulkanRHI/VulkanRHI.hxx"
#include "VulkanRHI/VulkanUtils.hxx"

namespace VulkanRHI
{

// Serialization

void ShaderResource::StageIO::Serialize(Serialization::StreamWriter* Writer, const ShaderResource::StageIO& Value)
{
    Writer->WriteString(Value.Name);
    Writer->WriteRaw(Value.Type);
    Writer->WriteRaw(Value.Binding);
    Writer->WriteRaw(Value.Location);
}

void ShaderResource::PushConstantRange::Serialize(Serialization::StreamWriter* Writer,
                                                  const ShaderResource::PushConstantRange& Value)
{
    Writer->WriteRaw(Value.Offset);
    Writer->WriteRaw(Value.Size);
    Writer->WriteObject(Value.Parameter);
}

void ShaderResource::StorageBuffer::Serialize(Serialization::StreamWriter* Writer,
                                              const ShaderResource::StorageBuffer& Value)
{
    Writer->WriteRaw(Value.Set);
    Writer->WriteRaw(Value.Binding);
    Writer->WriteObject(Value.Parameter);
}

void VulkanShader::ReflectionData::Serialize(Serialization::StreamWriter* Writer, const ReflectionData& Value)
{
    Writer->WriteArray<ShaderResource::StageIO>(Value.StageInput);
    Writer->WriteArray<ShaderResource::StageIO>(Value.StageOutput);
    Writer->WriteObject<ShaderResource::PushConstantRange>(Value.PushConstants);
    Writer->WriteArray<ShaderResource::StorageBuffer>(Value.StorageBuffers);
}

// Deserialization

void ShaderResource::StageIO::Deserialize(Serialization::StreamReader* Reader, ShaderResource::StageIO& OutValue)
{
    Reader->ReadString(OutValue.Name);
    Reader->ReadRaw(OutValue.Type);
    Reader->ReadRaw(OutValue.Binding);
    Reader->ReadRaw(OutValue.Location);
}

void ShaderResource::PushConstantRange::Deserialize(Serialization::StreamReader* Reader,
                                                    ShaderResource::PushConstantRange& OutValue)
{
    Reader->ReadRaw(OutValue.Offset);
    Reader->ReadRaw(OutValue.Size);
    Reader->ReadObject(OutValue.Parameter);
}

void ShaderResource::StorageBuffer::Deserialize(Serialization::StreamReader* Reader,
                                                ShaderResource::StorageBuffer& OutValue)
{
    Reader->ReadRaw(OutValue.Set);
    Reader->ReadRaw(OutValue.Binding);
    Reader->ReadObject(OutValue.Parameter);
}

void VulkanShader::ReflectionData::Deserialize(Serialization::StreamReader* Reader, ReflectionData& OutValue)
{
    Reader->ReadArray<ShaderResource::StageIO>(OutValue.StageInput);
    Reader->ReadArray<ShaderResource::StageIO>(OutValue.StageOutput);
    Reader->ReadObject<ShaderResource::PushConstantRange>(OutValue.PushConstants);
    Reader->ReadArray<ShaderResource::StorageBuffer>(OutValue.StorageBuffers);
}

Array<GraphicsPipelineDescription::VertexAttribute> VulkanShader::ReflectionData::GetInputVertexAttributes() const
{
    Array<GraphicsPipelineDescription::VertexAttribute> Result;
    Result.Reserve(StageInput.Size());

    uint32 Offset = 0;
    for (const ShaderResource::StageIO& Input: StageInput) {
        Result.Add(GraphicsPipelineDescription::VertexAttribute{
            .Location = Input.Location,
            .Binding = Input.Binding,
            .Format = Input.Type,
            .Offset = Offset,
        });
        Offset += GetSizeOfElementType(Input.Type);
    }
    return Result;
}

Array<GraphicsPipelineDescription::VertexBinding> VulkanShader::ReflectionData::GetInputVertexBindings() const
{
    Array<GraphicsPipelineDescription::VertexBinding> Result;
    uint32 Stride = 0;
    for (const ShaderResource::StageIO& Input: StageInput) {
        Stride += GetSizeOfElementType(Input.Type);
    }
    Result.Add(GraphicsPipelineDescription::VertexBinding{
        .Stride = Stride,
        .Binding = 0,
        .InputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    });
    return Result;
}

VulkanShader::VulkanShader(ERHIShaderType Type, const Array<uint32>& InSPIRVCode,
                           const ReflectionData& InReflectionData)
    : RHIShader(Type), SPIRVCode(InSPIRVCode), m_ReflectionData(InReflectionData), Type(Type)
{
    ShaderModuleCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .codeSize = SPIRVCode.Size() * sizeof(uint32),
        .pCode = SPIRVCode.Raw(),
    };
}

VulkanShader::~VulkanShader()
{
    for (VkDescriptorSetLayout& Layout: DescriptorSetLayout) {
        VulkanAPI::vkDestroyDescriptorSetLayout(GetVulkanDynamicRHI()->GetDevice()->GetHandle(), Layout,
                                                VULKAN_CPU_ALLOCATOR);
    }
}

const VkShaderModuleCreateInfo& VulkanShader::GetShaderModuleCreateInfo() const
{
    return ShaderModuleCreateInfo;
}

const char* VulkanShader::GetEntryPoint() const
{
    return "main";
}

Array<VkDescriptorSetLayout> VulkanShader::CompileDescriptorSetLayout()
{
    // Not the most efficient, but this will do for now
    for (uint32 Set = 0; Set < 32; Set += 1) {
        Array<VkDescriptorSetLayoutBinding> Bindings;
        for (const ShaderResource::StorageBuffer& Buffer: m_ReflectionData.StorageBuffers) {
            if (Buffer.Set != Set) {
                continue;
            }
            Bindings.Emplace(VkDescriptorSetLayoutBinding{
                .binding = Buffer.Binding,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = 1,
                .stageFlags = ConvertToVulkanType(GetShaderType()),
                .pImmutableSamplers = nullptr,
            });
        }

        if (Bindings.IsEmpty()) {
            continue;
        }
        VkDescriptorSetLayoutCreateInfo CreateInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .bindingCount = Bindings.Size(),
            .pBindings = Bindings.Raw(),
        };
        VK_CHECK_RESULT(VulkanAPI::vkCreateDescriptorSetLayout(GetVulkanDynamicRHI()->GetDevice()->GetHandle(),
                                                               &CreateInfo, VULKAN_CPU_ALLOCATOR,
                                                               &DescriptorSetLayout.Emplace()));
    }

    return DescriptorSetLayout;
}

}    // namespace VulkanRHI
