#pragma once

#include "Engine/Core/RHI/Resources/RHIShader.hxx"

#include "VulkanRHI/VulkanLoader.hxx"
#include "VulkanRHI/VulkanMemoryManager.hxx"

namespace VulkanRHI
{

class VulkanDevice;

namespace ShaderResource
{

    struct UniformBuffer {
        VkDescriptorBufferInfo Descriptor;
        uint32 Size = 0;
        uint32 BindingPoint = 0;
        std::string Name;
    };

    struct StorageBuffer {
        Ref<VulkanMemoryAllocation> MemoryAlloc;
        VkDescriptorBufferInfo Descriptor;
        uint32 Size = 0;
        uint32 BindingPoint = 0;
        std::string Name;
    };

    struct ImageSampler {
        uint32 BindingPoint = 0;
        uint32 DescriptorSet = 0;
        uint32 Dimension = 0;
        uint32 ArraySize = 0;
        std::string Name;
    };

    struct PushConstantRange {
        uint32 Offset = 0;
        uint32 Size = 0;
    };

}    // namespace ShaderResource

struct ShaderDescriptorSet {
    std::unordered_map<uint32, ShaderResource::UniformBuffer> UniformBuffers;
    std::unordered_map<uint32, ShaderResource::StorageBuffer> StorageBuffers;
    std::unordered_map<uint32, ShaderResource::ImageSampler> ImageSamplers;
    std::unordered_map<uint32, ShaderResource::ImageSampler> StorageImages;
    std::unordered_map<uint32, ShaderResource::ImageSampler> SeparateSamplers;

    std::unordered_map<std::string, VkWriteDescriptorSet> WriteDescriptorSets;

    operator bool() const
    {
        return !(StorageBuffers.empty() && UniformBuffers.empty() && ImageSamplers.empty() && StorageImages.empty() &&
                 SeparateSamplers.empty());
    }
};

class VulkanShader : public RHIShader
{
public:
    struct ReflectionData {
        Array<ShaderResource::PushConstantRange> PushConstants;
        std::unordered_map<std::string, ShaderDescriptorSet> DescriptorSets;
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

    Ref<ShaderHandle> GetHandle(Ref<VulkanDevice> InDevice);

private:
    const Array<uint32> SPIRVCode;
    const ReflectionData m_ReflectionData;

    WeakRef<ShaderHandle> m_ShaderHandle;

    friend class VulkanShaderCompiler;
};

}    // namespace VulkanRHI
