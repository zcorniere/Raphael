#include "VulkanRHI/Resources/VulkanBuffer.hxx"

#include "VulkanRHI/VulkanDevice.hxx"
#include "VulkanRHI/VulkanMemoryManager.hxx"

namespace VulkanRHI
{

static VkBufferUsageFlags ConvertToVulkanType(EBufferUsageFlags InUsage)
{
    VkBufferUsageFlags OutUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    auto TranslateFlags = [&OutUsage, &InUsage](EBufferUsageFlags SearchFlag, VkBufferUsageFlags AddedIfFound,
                                                VkBufferUsageFlags AddedIfNotFound = 0) {
        OutUsage |= EnumHasAnyFlags(InUsage, SearchFlag) ? AddedIfFound : AddedIfNotFound;
    };

    TranslateFlags(EBufferUsageFlags::VertexBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    TranslateFlags(EBufferUsageFlags::IndexBuffer, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    TranslateFlags(EBufferUsageFlags::StructuredBuffer, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    TranslateFlags(EBufferUsageFlags::SourceCopy, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    TranslateFlags(EBufferUsageFlags::DestinationCopy, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    TranslateFlags(EBufferUsageFlags::DrawIndirect, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);

    return OutUsage;
}

VulkanBuffer::VulkanBuffer(VulkanDevice* InDevice, const RHIBufferDesc& InDescription)
    : RHIBuffer(InDescription), IDeviceChild(InDevice)
{
    if (Description.Size == 0) {
        return;
    }

    check(Device);
    VkBufferCreateInfo CreateInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = Description.Size,
        .usage = ConvertToVulkanType(Description.Usage),
    };
    VmaAllocationCreateInfo AllocationInfo{
        .usage = VMA_MEMORY_USAGE_AUTO,
    };

    if (EnumHasAnyFlags(Description.Usage, EBufferUsageFlags::KeepCPUAccessible)) {
        AllocationInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                               VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT |
                               VMA_ALLOCATION_CREATE_MAPPED_BIT;
    }

    std::tie(BufferHandle, Memory) = Device->GetMemoryManager()->Alloc(CreateInfo, AllocationInfo);

    if (Description.ResourceArray) {
        if (!ensure(EnumHasAnyFlags(Description.Usage, EBufferUsageFlags::KeepCPUAccessible))) {
            return;
        }

        Memory->FlushMappedMemory(0, Description.ResourceArray->GetByteSize());
        void* const MappedPtr = Memory->Map(Description.ResourceArray->GetByteSize());
        std::memset(MappedPtr, 0, Description.Size);
        std::memcpy(MappedPtr, Description.ResourceArray->GetData(), Description.ResourceArray->GetByteSize());
        Memory->Unmap();
    }
}

VulkanBuffer::~VulkanBuffer()
{
    Device->GetMemoryManager()->Free(Memory);

    VulkanAPI::vkDestroyBuffer(Device->GetHandle(), BufferHandle, VULKAN_CPU_ALLOCATOR);
}

void VulkanBuffer::SetName(std::string_view InName)
{
    RObject::SetName(InName);
    VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_BUFFER, BufferHandle, "{:s}", InName);
    Memory->SetName(std::format("{:s}.Memory", InName));
}

}    // namespace VulkanRHI
