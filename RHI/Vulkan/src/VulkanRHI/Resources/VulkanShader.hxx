#pragma once

#include "Engine/Serialization/StreamReader.hxx"
#include "Engine/Serialization/StreamWriter.hxx"

#include "Engine/Core/RHI/RHIShaderParameters.hxx"
#include "Engine/Core/RHI/Resources/RHIShader.hxx"

#include "VulkanRHI/Resources/VulkanGraphicsPipeline.hxx"

namespace VulkanRHI
{

class FVulkanDevice;

namespace ShaderResource
{
    struct FPushConstantRange {
        uint32 Offset = 0;
        uint32 Size = 0;

        FShaderParameter Parameter;

        bool operator==(const FPushConstantRange&) const = default;

        static void Serialize(Serialization::FStreamWriter* Writer, const FPushConstantRange& Value);
        static void Deserialize(Serialization::FStreamReader* Reader, FPushConstantRange& OutValue);
    };

    struct FStageIO {
        std::string Name;
        EVertexElementType Type;
        uint32 Binding;
        uint32 Location;

        bool operator==(const FStageIO&) const = default;

        static void Serialize(Serialization::FStreamWriter* Writer, const FStageIO& Value);
        static void Deserialize(Serialization::FStreamReader* Reader, FStageIO& OutValue);
    };

    struct FStorageBuffer {
        uint32 Set = 0;
        uint32 Binding = 0;
        FShaderParameter Parameter;

        bool operator==(const FStorageBuffer&) const = default;

        static void Serialize(Serialization::FStreamWriter* Writer, const FStorageBuffer& Value);
        static void Deserialize(Serialization::FStreamReader* Reader, FStorageBuffer& OutValue);
    };

}    // namespace ShaderResource

class RVulkanShader : public RRHIShader
{
    RTTI_DECLARE_TYPEINFO(RVulkanShader, RRHIShader);

public:
    struct FReflectionData {
        TArray<ShaderResource::FStageIO> StageInput;
        TArray<ShaderResource::FStageIO> StageOutput;

        std::optional<ShaderResource::FPushConstantRange> PushConstants;

        TArray<ShaderResource::FStorageBuffer> StorageBuffers;

        TArray<FGraphicsPipelineDescription::FVertexBinding> GetInputVertexBindings() const;
        TArray<FGraphicsPipelineDescription::FVertexAttribute> GetInputVertexAttributes() const;

        bool operator==(const FReflectionData&) const = default;

        static void Serialize(Serialization::FStreamWriter* Writer, const FReflectionData& Value);
        static void Deserialize(Serialization::FStreamReader* Reader, FReflectionData& OutValue);
    };

public:
    RVulkanShader(ERHIShaderType Type, const TArray<uint32>& InSPRIVCode, const FReflectionData& InReflectionData);
    virtual ~RVulkanShader();

    const FReflectionData& GetReflectionData() const
    {
        return m_ReflectionData;
    }

    const VkShaderModuleCreateInfo& GetShaderModuleCreateInfo() const;
    const char* GetEntryPoint() const;

    constexpr ERHIShaderType GetShaderType() const
    {
        return Type;
    }

    TArray<VkDescriptorSetLayout> GetDescriptorSetLayout() const
    {
        return DescriptorSetLayout;
    }

    TArray<VkDescriptorSetLayout> CompileDescriptorSetLayout();

private:
    const TArray<uint32> SPIRVCode;
    const FReflectionData m_ReflectionData;

    ERHIShaderType Type;

    VkShaderModuleCreateInfo ShaderModuleCreateInfo;

    TArray<VkDescriptorSetLayout> DescriptorSetLayout;

    friend class VulkanShaderCompiler;
};

}    // namespace VulkanRHI

DEFINE_PRINTABLE_TYPE(VulkanRHI::ShaderResource::FPushConstantRange,
                      "PushConstantRange {{ Offset: {0}, Size: {1}, Parameter: {2:#} }}", Value.Offset, Value.Size,
                      Value.Parameter)

DEFINE_PRINTABLE_TYPE(VulkanRHI::ShaderResource::FStageIO,
                      "StageIO {{ Name: \"{0}\", Type: {1}, Binding: {2}, Location: {3} }}", Value.Name,
                      magic_enum::enum_name(Value.Type), Value.Binding, Value.Location)

DEFINE_PRINTABLE_TYPE(VulkanRHI::ShaderResource::FStorageBuffer,
                      "StorageBuffer {{ Set: {0}, Binding: {1}, Parameter: {2:#} }}", Value.Set, Value.Binding,
                      Value.Parameter)

DEFINE_PRINTABLE_TYPE(
    VulkanRHI::RVulkanShader::FReflectionData,
    "ReflectionData {{ StageInput: {0},\nStageOutput: {1},\nPushConstants: {2},\nStorageBuffers: {3} }}",
    Value.StageInput, Value.StageOutput,
    Value.PushConstants.has_value() ? Value.PushConstants.value() : VulkanRHI::ShaderResource::FPushConstantRange{},
    Value.StorageBuffers)
