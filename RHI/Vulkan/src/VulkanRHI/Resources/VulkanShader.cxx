#include "VulkanRHI/Resources/VulkanShader.hxx"

#include "Engine/Core/RHI/RHIDefinitions.hxx"
#include "VulkanRHI/VulkanUtils.hxx"

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

void RVulkanShader::FReflectionData::Deserialize(Serialization::FStreamReader* Reader, FReflectionData& OutValue)
{
    Reader->ReadArray<ShaderResource::FStageIO>(OutValue.StageInput);
    Reader->ReadArray<ShaderResource::FStageIO>(OutValue.StageOutput);
    Reader->ReadObject<ShaderResource::FPushConstantRange>(OutValue.PushConstants);
    Reader->ReadArray<ShaderResource::FStorageBuffer>(OutValue.StorageBuffers);
}

TArray<FGraphicsPipelineDescription::FVertexAttribute> RVulkanShader::FReflectionData::GetInputVertexAttributes() const
{
    TArray<FGraphicsPipelineDescription::FVertexAttribute> Result;
    Result.Reserve(StageInput.Size());

    uint32 Offset = 0;
    for (const ShaderResource::FStageIO& Input: StageInput) {
        Result.Add(FGraphicsPipelineDescription::FVertexAttribute{
            .Location = Input.Location,
            .Binding = Input.Binding,
            .Format = Input.Type,
            .Offset = Offset,
        });
        Offset += GetSizeOfElementType(Input.Type);
    }
    return Result;
}

TArray<FGraphicsPipelineDescription::FVertexBinding> RVulkanShader::FReflectionData::GetInputVertexBindings() const
{
    TArray<FGraphicsPipelineDescription::FVertexBinding> Result;
    uint32 Stride = 0;
    for (const ShaderResource::FStageIO& Input: StageInput) {
        Stride += GetSizeOfElementType(Input.Type);
    }
    Result.Add(FGraphicsPipelineDescription::FVertexBinding{
        .Stride = Stride,
        .Binding = 0,
        .InputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    });
    return Result;
}

RVulkanShader::RVulkanShader(ERHIShaderType Type, const TArray<uint32>& InSPIRVCode,
                             const FReflectionData& InReflectionData)
    : RRHIShader(Type), SPIRVCode(InSPIRVCode), m_ReflectionData(InReflectionData), Type(Type)
{
    ShaderModuleCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .codeSize = SPIRVCode.Size() * sizeof(uint32),
        .pCode = SPIRVCode.Raw(),
    };
}

RVulkanShader::~RVulkanShader()
{
}

const VkShaderModuleCreateInfo& RVulkanShader::GetShaderModuleCreateInfo() const
{
    return ShaderModuleCreateInfo;
}

const char* RVulkanShader::GetEntryPoint() const
{
    return "main";
}

bool RVulkanShader::GetDescriptorSetLayoutBindings(TArray<TArray<VkDescriptorSetLayoutBinding>>& OutBindings) const
{
    // Not the most efficient, but this will do for now
    for (const ShaderResource::FStorageBuffer& Buffer: m_ReflectionData.StorageBuffers) {
        if (Buffer.Set >= OutBindings.Size()) {
            OutBindings.Resize(Buffer.Set + 1);
        }
        VkDescriptorSetLayoutBinding* FoundBinding = OutBindings[Buffer.Set].FindByLambda(
            [Buffer](const VkDescriptorSetLayoutBinding& Binding) { return Binding.binding == Buffer.Binding; });
        if (FoundBinding) {
            FoundBinding->stageFlags |= ConvertToVulkanType(GetShaderType());
            continue;
        }

        VkDescriptorSetLayoutBinding Binding{
            .binding = Buffer.Binding,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1,
            .stageFlags = ConvertToVulkanType(GetShaderType()),
            .pImmutableSamplers = nullptr,
        };
        OutBindings[Buffer.Set].Add(Binding);
    }

    return true;
}

}    // namespace VulkanRHI
