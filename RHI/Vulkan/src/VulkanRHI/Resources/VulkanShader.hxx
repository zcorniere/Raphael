#pragma once

#include "Engine/Core/RHI/Resources/RHIShader.hxx"

#include "VulkanRHI/Resources/VulkanGraphicsPipeline.hxx"

namespace VulkanRHI
{

class VulkanDevice;

namespace ShaderResource
{
    struct PushConstantRange {
        uint32 Offset = 0;
        uint32 Size = 0;

        bool operator==(const PushConstantRange&) const = default;
    };

    struct StageIO {
        std::string Name;
        EVertexElementType Type;
        uint32 Binding;
        uint32 Location;

        bool operator==(const StageIO&) const = default;
    };

}    // namespace ShaderResource

class VulkanShader : public RHIShader
{
    RTTI_DECLARE_TYPEINFO(VulkanShader, RHIShader);

public:
    struct ReflectionData {
        Array<ShaderResource::StageIO> StageInput;
        Array<ShaderResource::StageIO> StageOutput;
        Array<ShaderResource::PushConstantRange> PushConstants;

        Array<GraphicsPipelineDescription::VertexBinding> GetInputVertexBindings() const;
        Array<GraphicsPipelineDescription::VertexAttribute> GetInputVertexAttributes() const;

        bool operator==(const ReflectionData&) const = default;
    };

    class ShaderHandle : public RObject, public IDeviceChild
    {
    public:
        ShaderHandle() = delete;
        ShaderHandle(VulkanDevice* InDevice, const VkShaderModuleCreateInfo& Info);
        ~ShaderHandle();

        virtual void SetName(std::string_view Name) override;

    public:
        VkShaderModule Handle;
    };

public:
    VulkanShader(ERHIShaderType Type, const Array<uint32>& InSPRIVCode, const ReflectionData& InReflectionData);

    virtual void SetName(std::string_view Name) override;

    const ReflectionData& GetReflectionData() const
    {
        return m_ReflectionData;
    }

    Ref<ShaderHandle> GetHandle(VulkanDevice* InDevice);

    constexpr ERHIShaderType GetShaderType() const
    {
        return Type;
    }

private:
    const Array<uint32> SPIRVCode;
    const ReflectionData m_ReflectionData;

    ERHIShaderType Type;

    Ref<ShaderHandle> m_ShaderHandle;

    friend class VulkanShaderCompiler;
};

}    // namespace VulkanRHI

DEFINE_PRINTABLE_TYPE(VulkanRHI::ShaderResource::PushConstantRange, "PushConstantRange {{ Offset: {0}, Size: {1} }}",
                      Value.Offset, Value.Size)

DEFINE_PRINTABLE_TYPE(VulkanRHI::ShaderResource::StageIO,
                      "StageIO {{ Name: \"{0}\", Type: {1}, Binding: {2}, Location: {3} }}", Value.Name,
                      magic_enum::enum_name(Value.Type), Value.Binding, Value.Location)
