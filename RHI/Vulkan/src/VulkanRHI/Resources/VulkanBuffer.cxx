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
        .usage = ConvertToVulkanType(Description.Usage, Description.Size == 0),
    };
    VK_CHECK_RESULT(VulkanAPI::vkCreateBuffer(Device->GetHandle(), &CreateInfo, VULKAN_CPU_ALLOCATOR, &BufferHandle));

    VulkanAPI::vkGetBufferMemoryRequirements(Device->GetHandle(), BufferHandle, &MemoryRequirements);
    Memory =
        Device->GetMemoryManager()->Alloc(MemoryRequirements, VMA_MEMORY_USAGE_CPU_TO_GPU,
                                          EnumHasAnyFlags(Description.Usage, EBufferUsageFlags::KeepCPUAccessible));
    Memory->BindBuffer(BufferHandle);
    if (Description.ResourceArray) {
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
