#include "VulkanRHI/DescriptorPoolManager.hxx"

#include "VulkanRHI/VulkanDevice.hxx"

namespace VulkanRHI
{

FDescriptorSetManager::FDescriptorSetManager(FVulkanDevice* InDevice)

    : IDeviceChild(InDevice)
{
}

FDescriptorSetManager::~FDescriptorSetManager()
{
    Destroy();
}

void FDescriptorSetManager::Initialize(const TArray<TArray<VkDescriptorSetLayoutBinding>>& InLayoutBindings,
                                       unsigned InMaxSets)
{
    CreateDescriptorSetLayout(InLayoutBindings);
    if (InLayoutBindings.IsEmpty()) {
        return;
    }

    CreateDescriptorPool(InLayoutBindings, InMaxSets);
    CreateDescriptorSets();
}

void FDescriptorSetManager::Destroy()
{
    for (VkDescriptorSet DescriptorSet: DescriptorSets) {
        VulkanAPI::vkFreeDescriptorSets(Device->GetHandle(), DescriptorPoolHandle, 1, &DescriptorSet);
    }
    DescriptorSets.Clear();

    VulkanAPI::vkDestroyDescriptorPool(Device->GetHandle(), DescriptorPoolHandle, VULKAN_CPU_ALLOCATOR);
    DescriptorPoolHandle = VK_NULL_HANDLE;

    for (VkDescriptorSetLayout Layout: DescriptorSetLayout) {
        VulkanAPI::vkDestroyDescriptorSetLayout(Device->GetHandle(), Layout, VULKAN_CPU_ALLOCATOR);
    }
    DescriptorSetLayout.Clear();
}

void FDescriptorSetManager::CreateDescriptorSetLayout(
    const TArray<TArray<VkDescriptorSetLayoutBinding>>& InLayoutBindings)
{
    DescriptorSetLayout.Resize(InLayoutBindings.Size());
    for (unsigned Set = 0; Set < InLayoutBindings.Size(); Set++) {
        const VkDescriptorSetLayoutCreateInfo CreateInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .bindingCount = InLayoutBindings[Set].Size(),
            .pBindings = InLayoutBindings[Set].Raw(),
        };
        VK_CHECK_RESULT(VulkanAPI::vkCreateDescriptorSetLayout(GetVulkanDynamicRHI()->GetDevice()->GetHandle(),
                                                               &CreateInfo, VULKAN_CPU_ALLOCATOR,
                                                               &DescriptorSetLayout[Set]));
    }
}

void FDescriptorSetManager::CreateDescriptorPool(const TArray<TArray<VkDescriptorSetLayoutBinding>>& InLayoutBindings,
                                                 unsigned InMaxSets)
{
    TArray<VkDescriptorPoolSize> PoolSizes;
    for (const TArray<VkDescriptorSetLayoutBinding>& InBinding: InLayoutBindings) {
        for (const VkDescriptorSetLayoutBinding& Binding: InBinding) {
            VkDescriptorPoolSize* const FoundPoolSize = PoolSizes.FindByLambda(
                [Binding](const VkDescriptorPoolSize& PoolSize) { return PoolSize.type == Binding.descriptorType; });
            if (FoundPoolSize) {
                FoundPoolSize->descriptorCount += Binding.descriptorCount;
                continue;
            }

            VkDescriptorPoolSize PoolSize{
                .type = Binding.descriptorType,
                .descriptorCount = Binding.descriptorCount,
            };
            PoolSizes.Emplace(PoolSize);
        }
    }

    VkDescriptorPoolCreateInfo CreateInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .maxSets = InMaxSets,
        .poolSizeCount = PoolSizes.Size(),
        .pPoolSizes = PoolSizes.Raw(),
    };
    VK_CHECK_RESULT(VulkanAPI::vkCreateDescriptorPool(Device->GetHandle(), &CreateInfo, VULKAN_CPU_ALLOCATOR,
                                                      &DescriptorPoolHandle));
}

void FDescriptorSetManager::CreateDescriptorSets()
{
    if (DescriptorSetLayout.IsEmpty()) {
        return;
    }

    VkDescriptorSetAllocateInfo AllocateInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = DescriptorPoolHandle,
        .descriptorSetCount = DescriptorSetLayout.Size(),
        .pSetLayouts = DescriptorSetLayout.Raw(),
    };
    DescriptorSets.Resize(DescriptorSetLayout.Size());
    VK_CHECK_RESULT(VulkanAPI::vkAllocateDescriptorSets(Device->GetHandle(), &AllocateInfo, DescriptorSets.Raw()));
}

}    // namespace VulkanRHI
