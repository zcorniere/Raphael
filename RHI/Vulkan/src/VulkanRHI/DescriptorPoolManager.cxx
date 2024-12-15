#include "VulkanRHI/DescriptorPoolManager.hxx"

#include "VulkanRHI/Resources/VulkanShader.hxx"
#include "VulkanRHI/VulkanDevice.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogDrescriptorSetManager, Warning)

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

bool FDescriptorSetManager::Initialize(const TArrayView<Ref<RVulkanShader>>& InShader, unsigned InMaxSets)
{
    TArray<TArray<VkDescriptorSetLayoutBinding>> DescriptorSetBindings;
    for (const Ref<RVulkanShader>& Shader: InShader) {
        if (!Shader->GetDescriptorSetLayoutBindings(DescriptorSetBindings))
            return false;
    }

    for (unsigned Set = 0; Set < DescriptorSetBindings.Size(); Set++) {
        for (const Ref<RVulkanShader>& Shader: InShader) {
            for (auto&& [Name, WriteDescriptor]: Shader->GetReflectionData().WriteDescriptorSet) {
                InputDeclaration[Name] = FRenderPassInputDeclaration{
                    .Type = ERenderPassInputType::StorageBuffer,
                    .Set = Set,
                    .Binding = WriteDescriptor.dstBinding,
                    .Count = WriteDescriptor.descriptorCount,
                    .Name = Name,
                };
                FRenderPassInput& Input = InputResource[Set][WriteDescriptor.dstBinding];
                Input.Type = ERenderPassInputType::StorageBuffer;    // @todo
                Input.Input.Resize(WriteDescriptor.descriptorCount);

                WriteDescriptorSet[Set][WriteDescriptor.dstBinding] = WriteDescriptor;
            }
        }
    }

    CreateDescriptorSetLayout(DescriptorSetBindings);
    if (DescriptorSetBindings.IsEmpty()) {
        return true;    // No descriptor set bindings, can happen so not an error
    }

    CreateDescriptorPool(DescriptorSetBindings, InMaxSets);
    CreateDescriptorSets();

    return true;
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

void FDescriptorSetManager::Bind(VkCommandBuffer CmdBuffer, VkPipelineLayout PipelineLayout,
                                 VkPipelineBindPoint BindPoint)
{
    if (DescriptorSets.IsEmpty()) {
        return;
    }

    TArray<VkWriteDescriptorSet> WriteDescriptorSetsArray;
    for (const auto& [Set, Bindings]: WriteDescriptorSet) {
        for (auto& [Binding, WriteDescriptor]: Bindings) {
            WriteDescriptorSetsArray.Emplace(WriteDescriptor);
            WriteDescriptorSetsArray.Back().dstSet = DescriptorSets[Set];

            Ref<RVulkanBuffer> Buffer = InputResource[Set][Binding].Input[0].As<RVulkanBuffer>();
            VkDescriptorBufferInfo Info{
                .buffer = Buffer->GetHandle(),
                .offset = 0,
                .range = Buffer->GetSize(),
            };
            WriteDescriptorSetsArray.Back().pBufferInfo = &Info;
        }
    }
    if (!WriteDescriptorSetsArray.IsEmpty()) {
        VulkanAPI::vkUpdateDescriptorSets(Device->GetHandle(), WriteDescriptorSetsArray.Size(),
                                          WriteDescriptorSetsArray.Raw(), 0, nullptr);
    }

    VulkanAPI::vkCmdBindDescriptorSets(CmdBuffer, BindPoint, PipelineLayout, 0, DescriptorSets.Size(),
                                       DescriptorSets.Raw(), 0, nullptr);
}

void FDescriptorSetManager::SetInput(std::string_view Name, const Ref<RVulkanBuffer>& Buffer)
{
    const FRenderPassInputDeclaration* const Declaration = GetInputDeclaration(Name);
    if (Declaration) {
        InputResource.at(Declaration->Set).at(Declaration->Binding).Set(Buffer);
    } else {
        LOG(LogDrescriptorSetManager, Warning, "Input declaration not found for {}", Name);
    }
}

const FDescriptorSetManager::FRenderPassInputDeclaration*
FDescriptorSetManager::GetInputDeclaration(std::string_view name) const
{
    std::string nameStr(name);
    std::unordered_map<std::string, FRenderPassInputDeclaration>::const_iterator it = InputDeclaration.find(nameStr);
    if (it == InputDeclaration.end())
        return nullptr;
    return &it->second;
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
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
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
        .descriptorSetCount = 1,
        .pSetLayouts = DescriptorSetLayout.Raw(),
    };
    DescriptorSets.Resize(DescriptorSetLayout.Size());
    VK_CHECK_RESULT(VulkanAPI::vkAllocateDescriptorSets(Device->GetHandle(), &AllocateInfo, DescriptorSets.Raw()));
}

}    // namespace VulkanRHI
