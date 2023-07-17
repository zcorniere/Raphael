#include "VulkanRHI/Resources/VulkanBuffer.hxx"

#include "VulkanRHI/VulkanDevice.hxx"
#include "VulkanRHI/VulkanMemoryManager.hxx"


namespace VulkanRHI
{

static VkBufferUsageFlags ConvertToVulkanType(EBufferUsageFlags InUsage, bool bZeroSize)
{
    VkBufferUsageFlags OutUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    auto TranslateFlags = [&OutUsage, &InUsage](EBufferUsageFlags SearchFlag, VkBufferUsageFlags AddedIfFound,
                                                VkBufferUsageFlags AddedIfNotFound = 0) {
        OutUsage |= EnumHasAnyFlags(InUsage, SearchFlag) ? AddedIfFound : AddedIfNotFound;
    };

    TranslateFlags(EBufferUsageFlags::VertexBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    TranslateFlags(EBufferUsageFlags::IndexBuffer, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    TranslateFlags(EBufferUsageFlags::StructuredBuffer, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    if (!bZeroSize) {
        TranslateFlags(EBufferUsageFlags::DrawIndirect, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);
    }
    return OutUsage;
}

VulkanBuffer::VulkanBuffer(VulkanDevice* InDevice, const uint32 InSize, const EBufferUsageFlags InUsage,
                           const uint32 InStride, Ref<ResourceArray>& InitialData)
    : RHIBuffer(InUsage, InSize, InStride),
      Device(InDevice),
      Offset(0),
      BufferUsageFlags(ConvertToVulkanType(InUsage, InSize == 0))
{
    if (InSize == 0) {
        return;
    }

    check(Device);
    VkBufferCreateInfo CreateInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = InSize,
        .usage = BufferUsageFlags,
    };
    VK_CHECK_RESULT(VulkanAPI::vkCreateBuffer(Device->GetHandle(), &CreateInfo, VULKAN_CPU_ALLOCATOR, &BufferHandle));

    VulkanAPI::vkGetBufferMemoryRequirements(Device->GetHandle(), BufferHandle, &MemoryRequirements);
    Memory = Device->GetMemoryManager()->Alloc(MemoryRequirements, VMA_MEMORY_USAGE_GPU_ONLY, false);
    Memory->BindBuffer(BufferHandle);

    if (InitialData) {
        uint32 CopyDataSize = std::min(InSize, InitialData->GetResourceDataSize());

        void* Data = Memory->Map(CopyDataSize);
        std::memcpy(Data, InitialData->GetResourceData(), CopyDataSize);
        Memory->FlushMappedMemory(CopyDataSize, 0);
        Memory->Unmap();
        InitialData->Discard();
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
    VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_BUFFER, BufferHandle, "{}", InName);
    Memory->SetName(std::format("{} [Memory]", InName));
}

uint64 VulkanBuffer::GetCurrentSize() const
{
    return Memory->GetSize() - Offset;
}

}    // namespace VulkanRHI
