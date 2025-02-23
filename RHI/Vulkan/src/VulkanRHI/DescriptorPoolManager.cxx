#include "VulkanRHI/DescriptorPoolManager.hxx"

#include "VulkanRHI/Resources/VulkanShader.hxx"
#include "VulkanRHI/VulkanDevice.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogDescriptorSetManager, Warning)

namespace VulkanRHI
{

FDescriptorSetManager::FDescriptorSetManager(FVulkanDevice* InDevice, const TArray<WeakRef<RVulkanShader>>& InShaders)
    : IDeviceChild(InDevice), Shaders(InShaders)
{
    unsigned MaxSets = 0;
    for (const WeakRef<RVulkanShader>& Shader: Shaders) {
        const TArray<VkDescriptorSetLayout>& DescriptorSetLayouts = Shader->GetDescriptorSetLayout();
        MaxSets = std::max(MaxSets, DescriptorSetLayouts.Size());
    }

    for (unsigned Set = 0; Set < MaxSets; Set++) {
        for (const WeakRef<RVulkanShader>& Shader: Shaders) {
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
}

FDescriptorSetManager::~FDescriptorSetManager()
{
}

void FDescriptorSetManager::Destroy()
{
    if (DescriptorSets.IsEmpty() && DescriptorPoolHandle != VK_NULL_HANDLE) {
        RHI::DeferedDeletion([Handle = DescriptorPoolHandle, Device = Device] {
            VulkanAPI::vkDestroyDescriptorPool(Device->GetHandle(), Handle, VULKAN_CPU_ALLOCATOR);
        });
    }
    DescriptorSets.Clear();

    if (DescriptorPoolHandle != VK_NULL_HANDLE) {
        RHI::DeferedDeletion([Handle = DescriptorPoolHandle, Device = Device] {
            VulkanAPI::vkDestroyDescriptorPool(Device->GetHandle(), Handle, VULKAN_CPU_ALLOCATOR);
        });
    }
    DescriptorPoolHandle = VK_NULL_HANDLE;
}

void FDescriptorSetManager::Bake()
{
    TArray<VkDescriptorSetLayout> DescriptorSetLayouts;
    TArray<VkDescriptorPoolSize> PoolSizes;
    for (const WeakRef<RVulkanShader>& Shader: Shaders) {
        DescriptorSetLayouts.Append(Shader->GetDescriptorSetLayout());
        PoolSizes.Append(Shader->GetDescriptorPoolSizes());
    }

    CreateDescriptorPool(PoolSizes, 1);
    CreateDescriptorSets(DescriptorSetLayouts);

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
}

void FDescriptorSetManager::Bind(VkCommandBuffer CmdBuffer, VkPipelineLayout PipelineLayout,
                                 VkPipelineBindPoint BindPoint)
{
    VulkanAPI::vkCmdBindDescriptorSets(CmdBuffer, BindPoint, PipelineLayout, 0, DescriptorSets.Size(),
                                       DescriptorSets.Raw(), 0, nullptr);
}

void FDescriptorSetManager::InvalidateAndUpdate()
{
    std::unordered_map<uint32, std::unordered_map<uint32, FRenderPassInput>> InvalidatedInput;

    for (auto& [Set, Inputs]: InputResource) {
        for (auto& [Binding, Input]: Inputs) {
            switch (Input.Type) {
                case ERenderPassInputType::StorageBuffer: {
                    const VkDescriptorBufferInfo& Info = Input.Input[0].As<RVulkanBuffer>()->GetDescriptorBufferInfo();
                    const VkWriteDescriptorSet& SetWrite = WriteDescriptorSet.at(Set).at(Binding);
                    if (SetWrite.pBufferInfo && Info.buffer != SetWrite.pBufferInfo->buffer) {
                        InvalidatedInput[Set][Binding] = Input;
                    }
                } break;
                default:
                    break;
            }
        }
    }

    if (InvalidatedInput.empty())
        return;

    for (auto& [Set, Inputs]: InvalidatedInput) {

        TArray<VkWriteDescriptorSet> WriteDescriptorSetsToUpdate;
        for (auto& [Binding, Input]: Inputs) {
            VkWriteDescriptorSet& WriteDescriptor = WriteDescriptorSet.at(Set).at(Binding);
            switch (Input.Type) {
                case ERenderPassInputType::StorageBuffer: {
                    const VkDescriptorBufferInfo& Info = Input.Input[0].As<RVulkanBuffer>()->GetDescriptorBufferInfo();
                    WriteDescriptor.pBufferInfo = &Info;
                } break;
                default:
                    break;
            }
            WriteDescriptorSetsToUpdate.Emplace(WriteDescriptor);
        }

        VulkanAPI::vkUpdateDescriptorSets(Device->GetHandle(), WriteDescriptorSetsToUpdate.Size(),
                                          WriteDescriptorSetsToUpdate.Raw(), 0, nullptr);
    }
}

void FDescriptorSetManager::SetInput(std::string_view Name, const Ref<RVulkanBuffer>& Buffer)
{
    const FRenderPassInputDeclaration* const Declaration = GetInputDeclaration(Name);
    if (Declaration) {
        InputResource.at(Declaration->Set).at(Declaration->Binding).Set(Buffer);
    } else {
        LOG(LogDescriptorSetManager, Warning, "Input declaration not found for {}", Name);
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

void FDescriptorSetManager::CreateDescriptorPool(const TArray<VkDescriptorPoolSize>& PoolSize, unsigned InMaxSets)
{
    VkDescriptorPoolCreateInfo CreateInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = InMaxSets,
        .poolSizeCount = PoolSize.Size(),
        .pPoolSizes = PoolSize.Raw(),
    };
    ensure(DescriptorPoolHandle == VK_NULL_HANDLE);
    VK_CHECK_RESULT(VulkanAPI::vkCreateDescriptorPool(Device->GetHandle(), &CreateInfo, VULKAN_CPU_ALLOCATOR,
                                                      &DescriptorPoolHandle));
}

void FDescriptorSetManager::CreateDescriptorSets(const TArray<VkDescriptorSetLayout>& DescriptorSetLayouts)
{
    VkDescriptorSetAllocateInfo AllocateInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = DescriptorPoolHandle,
        .descriptorSetCount = 1,
        .pSetLayouts = DescriptorSetLayouts.Raw(),
    };
    ensure(DescriptorSets.IsEmpty());

    DescriptorSets.Resize(1);
    VK_CHECK_RESULT(VulkanAPI::vkAllocateDescriptorSets(Device->GetHandle(), &AllocateInfo, DescriptorSets.Raw()));
}

}    // namespace VulkanRHI
