#pragma once

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

class VulkanShader : public RHIShader
{
public:
    struct ReflectionData {
        Array<ShaderResource::StageIO> StageInput;
        Array<ShaderResource::StageIO> StageOutput;
        Array<ShaderResource::PushConstantRange> PushConstants;

        bool operator==(const ReflectionData&) const = default;
    };

    class ShaderHandle : public RObject
    {
    public:
        ShaderHandle() = delete;
        ShaderHandle(Ref<VulkanDevice>& InDevice, const VkShaderModuleCreateInfo& Info);
        ~ShaderHandle();

        VkShaderModule Handle;

    private:
        Ref<VulkanDevice> Device;
    };

public:
    VulkanShader(RHIShaderType Type, const Array<uint32>& InSPRIVCode, const ReflectionData& InReflectionData);

    const ReflectionData& GetReflectionData() const
    {
        return m_ReflectionData;
    }

    Ref<ShaderHandle> GetHandle(Ref<VulkanDevice> InDevice);

private:
    const Array<uint32> SPIRVCode;
    const ReflectionData m_ReflectionData;

    WeakRef<ShaderHandle> m_ShaderHandle;

    friend class VulkanShaderCompiler;
};

}    // namespace VulkanRHI
