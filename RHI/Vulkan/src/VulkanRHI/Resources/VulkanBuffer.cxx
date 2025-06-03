#include "VulkanRHI/Resources/VulkanBuffer.hxx"

#include "VulkanRHI/VulkanDevice.hxx"
#include "VulkanRHI/VulkanMemoryManager.hxx"

namespace VulkanRHI
{

static VkBufferUsageFlags ConvertToVulkanType(EBufferUsageFlags InUsage)
{
    VkBufferUsageFlags OutUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    auto TranslateFlags = [&OutUsage, &InUsage](EBufferUsageFlags SearchFlag, VkBufferUsageFlags AddedIfFound,
                                                VkBufferUsageFlags AddedIfNotFound = 0)
    { OutUsage |= EnumHasAnyFlags(InUsage, SearchFlag) ? AddedIfFound : AddedIfNotFound; };

    TranslateFlags(EBufferUsageFlags::VertexBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    TranslateFlags(EBufferUsageFlags::IndexBuffer, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    TranslateFlags(EBufferUsageFlags::StorageBuffer, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    TranslateFlags(EBufferUsageFlags::UniformBuffer, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    TranslateFlags(EBufferUsageFlags::SourceCopy, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    TranslateFlags(EBufferUsageFlags::DestinationCopy, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    TranslateFlags(EBufferUsageFlags::DrawIndirect, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);

    return OutUsage;
}

RVulkanBuffer::RVulkanBuffer(FVulkanDevice* InDevice, const FRHIBufferDesc& InDescription)
    : Super(InDescription)
    , IDeviceChild(InDevice)
{

    check(Device);
    VkBufferCreateInfo CreateInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = Description.Size,
        .usage = ConvertToVulkanType(Description.Usage),
    };
    VmaAllocationCreateInfo AllocationInfo{
        .usage = VMA_MEMORY_USAGE_AUTO,
    };

    if (EnumHasAnyFlags(Description.Usage, EBufferUsageFlags::KeepCPUAccessible))
    {
        AllocationInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                               VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT |
                               VMA_ALLOCATION_CREATE_MAPPED_BIT;
    }

    if (CreateInfo.size == 0 && Description.ResourceArray)
    {
        CreateInfo.size = Description.ResourceArray->GetByteSize();
    }
    checkMsg(CreateInfo.size > 0, "Buffer size must be greater than 0");
    std::tie(BufferHandle, Memory) = Device->GetMemoryManager()->Alloc(CreateInfo, AllocationInfo);

    BufferInfo = {
        .buffer = BufferHandle,
        .offset = 0,
        .range = Description.Size,
    };

    if (Description.ResourceArray)
    {
        ensure(Description.ResourceArray->GetByteSize() <= Description.Size);
        if (!ensure(EnumHasAnyFlags(Description.Usage, EBufferUsageFlags::KeepCPUAccessible)))
        {
            return;
        }

        void* const MappedPtr = Memory->Map(Description.ResourceArray->GetByteSize());
        std::memcpy(MappedPtr, Description.ResourceArray->GetData(), Description.ResourceArray->GetByteSize());
        Memory->Unmap();
        Memory->FlushMappedMemory(0, InDescription.ResourceArray->GetByteSize());
    }
}

RVulkanBuffer::~RVulkanBuffer()
{
    RHI::DeferedDeletion(
        [Memory = this->Memory, BufferHandle = this->BufferHandle, Device = this->Device]() mutable
        {
            Device->GetMemoryManager()->Free(Memory);

            VulkanAPI::vkDestroyBuffer(Device->GetHandle(), BufferHandle, VULKAN_CPU_ALLOCATOR);
        });
}

void RVulkanBuffer::SetName(std::string_view InName)
{
    Super::SetName(InName);
    VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_BUFFER, BufferHandle, "{:s}", InName);
    Memory->SetName(std::format("{:s}.Memory", InName));
}

}    // namespace VulkanRHI
