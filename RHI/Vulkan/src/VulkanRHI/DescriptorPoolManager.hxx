#pragma once

#include "Engine/Core/RHI/RHIResource.hxx"
#include "VulkanRHI/Resources/VulkanBuffer.hxx"

namespace VulkanRHI
{
class RVulkanShader;

class FDescriptorSetManager : public IDeviceChild
{
public:
    enum class ERenderPassInputType : uint16_t {
        None = 0,
        StorageBuffer,
    };

    struct FRenderPassInputDeclaration {
        ERenderPassInputType Type = ERenderPassInputType::None;
        uint32_t Set = 0;
        uint32_t Binding = 0;
        uint32_t Count = 0;
        std::string Name;
    };

    struct FRenderPassInput {
        FRenderPassInput() = default;
        FRenderPassInput(Ref<RVulkanBuffer> StorageBuffer): Type(ERenderPassInputType::StorageBuffer)
        {
            Input.Add(StorageBuffer);
        }

        void Set(Ref<RVulkanBuffer> StorageBuffer, uint32_t index = 0)
        {
            Type = ERenderPassInputType::StorageBuffer;
            Input[index] = StorageBuffer;
        }

        ERenderPassInputType Type = ERenderPassInputType::None;
        TArray<Ref<RRHIResource>> Input;
    };

public:
    FDescriptorSetManager(FVulkanDevice* InDevice);
    virtual ~FDescriptorSetManager();

    bool Initialize(const TArrayView<Ref<RVulkanShader>>& InShader, unsigned InMaxSets);
    void Destroy();

    void Bind(VkCommandBuffer Cmd, VkPipelineLayout PipelineLayout, VkPipelineBindPoint BindPoint);

    void SetInput(std::string_view Name, const Ref<RVulkanBuffer>& Buffer);

    VkDescriptorPool GetHandle() const
    {
        return DescriptorPoolHandle;
    }
    VkDescriptorSet GetDescriptorSet(unsigned Set) const;
    const TArray<VkDescriptorSetLayout>& GetDescriptorSetLayout() const
    {
        return DescriptorSetLayout;
    }

    const FRenderPassInputDeclaration* GetInputDeclaration(std::string_view name) const;

private:
    void CreateDescriptorSetLayout(const TArray<TArray<VkDescriptorSetLayoutBinding>>& InLayoutBindings);
    void CreateDescriptorPool(const TArray<TArray<VkDescriptorSetLayoutBinding>>& InLayoutBindings, unsigned InMaxSets);
    void CreateDescriptorSets();

private:
    VkDescriptorPool DescriptorPoolHandle = VK_NULL_HANDLE;

    TArray<VkDescriptorSetLayout> DescriptorSetLayout = {};
    TArray<VkDescriptorSet> DescriptorSets = {};

    std::unordered_map<std::string, FRenderPassInputDeclaration> InputDeclaration = {};
    std::unordered_map<uint32, std::unordered_map<uint32, FRenderPassInput>> InputResource = {};

    std::unordered_map<uint32_t, std::unordered_map<uint32_t, VkWriteDescriptorSet>> WriteDescriptorSet = {};
};

}    // namespace VulkanRHI
