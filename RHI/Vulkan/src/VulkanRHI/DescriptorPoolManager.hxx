#pragma once

namespace VulkanRHI
{

class FDescriptorSetManager : public IDeviceChild
{
public:
    FDescriptorSetManager(FVulkanDevice* InDevice);
    ~FDescriptorSetManager();

    void Initialize(const TArray<TArray<VkDescriptorSetLayoutBinding>>& InLayoutBindings, unsigned InMaxSets);
    void Destroy();

    VkDescriptorPool GetHandle() const
    {
        return DescriptorPoolHandle;
    }
    VkDescriptorSet GetDescriptorSet(unsigned Set) const;
    const TArray<VkDescriptorSetLayout>& GetDescriptorSetLayout() const
    {
        return DescriptorSetLayout;
    }

private:
    void CreateDescriptorSetLayout(const TArray<TArray<VkDescriptorSetLayoutBinding>>& InLayoutBindings);
    void CreateDescriptorPool(const TArray<TArray<VkDescriptorSetLayoutBinding>>& InLayoutBindings, unsigned InMaxSets);
    void CreateDescriptorSets();

private:
    VkDescriptorPool DescriptorPoolHandle = VK_NULL_HANDLE;

    TArray<VkDescriptorSetLayout> DescriptorSetLayout = {};
    TArray<VkDescriptorSet> DescriptorSets = {};
};

}    // namespace VulkanRHI
