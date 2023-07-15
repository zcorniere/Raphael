#pragma once

#include "Engine/Core/RHI/RHIDefinitions.hxx"
#include "Engine/Core/RHI/Resources/RHIShader.hxx"

#include "VulkanRHI/VulkanLoader.hxx"

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
        uint32 Location;

        bool operator==(const StageIO&) const = default;
    };

}    // namespace ShaderResource

class VulkanShader final : public RHIShader
{
public:
    struct ReflectionData {
        Array<ShaderResource::StageIO> StageInput;
        Array<ShaderResource::StageIO> StageOutput;
        Array<ShaderResource::PushConstantRange> PushConstants;

        bool operator==(const ReflectionData&) const = default;
    };

    class ShaderHandle
    {
    public:
        ShaderHandle() = delete;
        ShaderHandle(VulkanDevice* InDevice, const VkShaderModuleCreateInfo& Info);
        ~ShaderHandle();

        VkShaderModule Handle;

    private:
        VulkanDevice* Device;
    };

public:
    VulkanShader(RHIShaderType Type, const Array<uint32>& InSPRIVCode, const ReflectionData& InReflectionData);
    ~VulkanShader();

    const ReflectionData& GetReflectionData() const
    {
        return m_ReflectionData;
    }

    ShaderHandle* GetHandle(VulkanDevice* InDevice);

private:
    const Array<uint32> SPIRVCode;
    const ReflectionData m_ReflectionData;

    ShaderHandle* m_ShaderHandle;

    friend class VulkanShaderCompiler;
};

}    // namespace VulkanRHI

DEFINE_PRINTABLE_TYPE(VulkanRHI::ShaderResource::PushConstantRange, "{{ Offset: {0}, Size: {1} }}", Value.Offset,
                      Value.Size)

DEFINE_PRINTABLE_TYPE(VulkanRHI::ShaderResource::StageIO, "{{ Name: {0}, Type: {1}, Location: {2} }}", Value.Name,
                      magic_enum::enum_name(Value.Type), Value.Location)
