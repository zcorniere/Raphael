#include "VulkanRHI/Resources/VulkanShader.hxx"

#include "Engine/Core/RHI/RHIDefinitions.hxx"
#include "VulkanRHI/VulkanDevice.hxx"

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
    Writer->WriteArray<ShaderResource::PushConstantRange>(Value.PushConstants);
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
    Reader->ReadArray<ShaderResource::PushConstantRange>(OutValue.PushConstants);
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

VulkanShader::ShaderHandle::ShaderHandle(VulkanDevice* InDevice, const VkShaderModuleCreateInfo& Info)
    : IDeviceChild(InDevice)
{
    VK_CHECK_RESULT(VulkanAPI::vkCreateShaderModule(Device->GetHandle(), &Info, VULKAN_CPU_ALLOCATOR, &Handle));
}

VulkanShader::ShaderHandle::~ShaderHandle()
{
    VulkanAPI::vkDestroyShaderModule(Device->GetHandle(), Handle, VULKAN_CPU_ALLOCATOR);
}

void VulkanShader::ShaderHandle::SetName(std::string_view Name)
{
    RObject::SetName(Name);
    VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_SHADER_MODULE, Handle, "{:s}", Name);
}

VulkanShader::VulkanShader(ERHIShaderType Type, const Array<uint32>& InSPIRVCode,
                           const ReflectionData& InReflectionData)
    : RHIShader(Type), SPIRVCode(InSPIRVCode), m_ReflectionData(InReflectionData), Type(Type), m_ShaderHandle(nullptr)
{
}

VulkanShader::~VulkanShader()
{
}

void VulkanShader::SetName(std::string_view Name)
{
    RObject::SetName(Name);
    if (m_ShaderHandle) {
        m_ShaderHandle->SetName(Name);
    }
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
    m_ShaderHandle->SetName(GetName());
    return m_ShaderHandle;
}

}    // namespace VulkanRHI
