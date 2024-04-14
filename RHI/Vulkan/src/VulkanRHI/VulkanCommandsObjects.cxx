#include "VulkanRHI/VulkanCommandsObjects.hxx"

#include "VulkanRHI/RenderPass/VulkanRenderPass.hxx"
#include "VulkanRHI/VulkanDevice.hxx"
#include "VulkanRHI/VulkanQueue.hxx"

#include "VulkanRHI/Resources/VulkanGraphicsPipeline.hxx"

namespace VulkanRHI
{

/// ------------------- VulkanCmdBuffer -------------------
VulkanCmdBuffer::VulkanCmdBuffer(VulkanDevice* InDevice, VulkanCommandBufferPool* InCommandPool)
    : IDeviceChild(InDevice), State(EState::NotAllocated), m_OwnerPool(InCommandPool)
{
    Allocate();

    m_Fence = Ref<Fence>::Create(Device, false);
}

VulkanCmdBuffer::~VulkanCmdBuffer()
{
    if (State == EState::Submitted) {
        LOG(LogVulkanRHI, Warning,
            "Attempting to destroy a buffer still in flight ! Waiting 16ms so it can be destroyed");
        // Wait 16 ms
        m_Fence->Wait(16 * 1000 * 1000LL);
        m_Fence->Reset();
    }

    if (State != EState::NotAllocated) {
        Free();
    }
}

void VulkanCmdBuffer::SetName(std::string_view InName)
{
    NamedClass::SetName(InName);
    if (m_CommandBufferHandle) {
        VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_COMMAND_BUFFER, m_CommandBufferHandle, "{:s}", InName);
    }
    if (m_Fence) {
        m_Fence->SetName(InName);
    }
}

void VulkanCmdBuffer::Begin()
{
    if (State == EState::NeedReset) {
        VulkanAPI::vkResetCommandBuffer(m_CommandBufferHandle, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
    } else {
        checkMsg(State == EState::ReadyForBegin, "Can't Begin as we're NOT ready! CmdBuffer {:p} State={:s}",
                 (void*)m_CommandBufferHandle, magic_enum::enum_name(State));
    }
    State = EState::IsInsideBegin;

    VkCommandBufferBeginInfo CmdBufBeginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    VK_CHECK_RESULT(VulkanAPI::vkBeginCommandBuffer(m_CommandBufferHandle, &CmdBufBeginInfo));
}

void VulkanCmdBuffer::End()
{
    checkMsg(IsOutsideRenderPass(), "Can't End as we're inside a render pass! CmdBuffer {:p} State={:s}",
             (void*)m_CommandBufferHandle, magic_enum::enum_name(State));

    VK_CHECK_RESULT(VulkanAPI::vkEndCommandBuffer(m_CommandBufferHandle));
    State = EState::HasEnded;
}

void VulkanCmdBuffer::BeginRenderPass(const VkRenderPassBeginInfo& RenderPassBeginInfo)
{
    State = EState::IsInsideRenderPass;
    VulkanAPI::vkCmdBeginRenderPass(m_CommandBufferHandle, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanCmdBuffer::EndRenderPass()
{
    checkMsg(IsInsideRenderPass(), "Can't EndRenderPass as we're NOT inside one! CmdBuffer {:p} State={:s}",
             (void*)m_CommandBufferHandle, magic_enum::enum_name(State));
    VulkanAPI::vkCmdEndRenderPass(m_CommandBufferHandle);
    State = EState::IsInsideBegin;
}

void VulkanCmdBuffer::AddWaitSemaphore(VkPipelineStageFlags InWaitFlags, Ref<Semaphore> InSemaphore)
{
    if (!WaitFlags.Contains(InWaitFlags)) {
        WaitFlags.Add(InWaitFlags);
    }
    WaitSemaphore.Add(InSemaphore);
}

void VulkanCmdBuffer::Allocate()
{
    check(State == EState::NotAllocated);

    VkCommandBufferAllocateInfo CreateCmdBufInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = m_OwnerPool->GetHandle(),
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    VK_CHECK_RESULT(
        VulkanAPI::vkAllocateCommandBuffers(Device->GetHandle(), &CreateCmdBufInfo, &m_CommandBufferHandle));
    State = EState::ReadyForBegin;
}

void VulkanCmdBuffer::Free()
{
    check(State != EState::NotAllocated);
    check(m_CommandBufferHandle != VK_NULL_HANDLE);

    VulkanAPI::vkFreeCommandBuffers(Device->GetHandle(), m_OwnerPool->GetHandle(), 1, &m_CommandBufferHandle);

    m_CommandBufferHandle = VK_NULL_HANDLE;
    State = EState::NotAllocated;
}

void VulkanCmdBuffer::RefreshFenceStatus()
{
    if (State != EState::Submitted) {
        check(!m_Fence->IsSignaled());
        return;
    }

    if (m_Fence->IsSignaled()) {
        WaitSemaphore.Clear();

        m_Fence->Reset();
        State = EState::NeedReset;
    }
}

/// ------------------- VulkanCommandBufferPool -------------------

VulkanCommandBufferPool::VulkanCommandBufferPool(VulkanDevice* InDevice)
    : IDeviceChild(InDevice), m_Handle(VK_NULL_HANDLE)
{
}

VulkanCommandBufferPool::~VulkanCommandBufferPool()
{
    m_CmdBuffers.Clear(true);

    VulkanAPI::vkDestroyCommandPool(Device->GetHandle(), m_Handle, VULKAN_CPU_ALLOCATOR);
    m_Handle = VK_NULL_HANDLE;
}

void VulkanCommandBufferPool::SetName(std::string_view InName)
{
    NamedClass::SetName(InName);
    if (m_Handle) {
        VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_COMMAND_POOL, m_Handle, "{:s}", InName);
    }
}

void VulkanCommandBufferPool::Initialize(uint32 QueueFamilyIndex)
{
    VkCommandPoolCreateInfo CmdPoolInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = QueueFamilyIndex,
    };
    VK_CHECK_RESULT(VulkanAPI::vkCreateCommandPool(Device->GetHandle(), &CmdPoolInfo, VULKAN_CPU_ALLOCATOR, &m_Handle));
}

VulkanCmdBuffer* VulkanCommandBufferPool::GetCommandBuffer()
{
    // Find already allocated buffer ...
    for (uint32 Index = 0; Index < m_CmdBuffers.Size(); Index++) {
        VulkanCmdBuffer* const CmdBuffer = m_CmdBuffers[Index];
        CmdBuffer->RefreshFenceStatus();

        if (CmdBuffer->State == VulkanCmdBuffer::EState::ReadyForBegin ||
            CmdBuffer->State == VulkanCmdBuffer::EState::NeedReset) {
            return CmdBuffer;
        }
    }

    // No buffer are available, create a new one. It already has memory
    VulkanCmdBuffer* const NewCmdBuffer = new VulkanCmdBuffer(Device, this);
    m_CmdBuffers.Add(NewCmdBuffer);
    return NewCmdBuffer;
}

void VulkanCommandBufferPool::RefreshFenceStatus(const VulkanCmdBuffer* SkipCmdBuffer)
{
    for (VulkanCmdBuffer* const CmdBuffer: m_CmdBuffers) {
        if (CmdBuffer == SkipCmdBuffer)
            continue;
        CmdBuffer->RefreshFenceStatus();
    }
}

/// ------------------- VulkanCommandBufferManager -------------------

VulkanCommandBufferManager::VulkanCommandBufferManager(VulkanDevice* InDevice, VulkanQueue* InQueue)
    : IDeviceChild(InDevice), Queue(InQueue)
{
    Pool = new VulkanCommandBufferPool(Device);
    Pool->Initialize(Queue->GetFamilyIndex());
    Pool->SetName("Main.CommandPool");

    ActiveCmdBufferRef = Pool->GetCommandBuffer();
    ActiveCmdBufferRef->SetName(std::format("Active{:d}.CommandBuffer", GFrameCounter));
    UploadCmdBufferRef = nullptr;
}

VulkanCommandBufferManager::~VulkanCommandBufferManager()
{
    delete Pool;
}

VulkanCmdBuffer* VulkanCommandBufferManager::GetActiveCmdBuffer()
{
    if (UploadCmdBufferRef) {
        SubmitUploadCmdBuffer();
    }
    return ActiveCmdBufferRef;
}
VulkanCmdBuffer* VulkanCommandBufferManager::GetUploadCmdBuffer()
{
    if (!UploadCmdBufferRef) {
        UploadCmdBufferRef = FindAvailableCmdBuffer();
        UploadCmdBufferRef->SetName("Upload.CommandBuffer");
    }
    return UploadCmdBufferRef;
}

void VulkanCommandBufferManager::PrepareForNewActiveCommandBuffer()
{
    ActiveCmdBufferRef = FindAvailableCmdBuffer();
    ActiveCmdBufferRef->SetName(std::format("Active{:d}.CommandBuffer", GFrameCounter));
}

void VulkanCommandBufferManager::SubmitUploadCmdBuffer(Ref<Semaphore> SignalSemaphore)
{
    check(UploadCmdBufferRef);

    if (!UploadCmdBufferRef->IsSubmitted() && UploadCmdBufferRef->HasBegun()) {
        check(UploadCmdBufferRef->IsOutsideRenderPass());

        UploadCmdBufferRef->End();

        if (SignalSemaphore) {
            Queue->Submit(UploadCmdBufferRef, SignalSemaphore->GetHandle());
        } else {
            Queue->Submit(UploadCmdBufferRef);
        }
    }
    UploadCmdBufferRef->SetName("Unused.Buffer");
    UploadCmdBufferRef = nullptr;
}

void VulkanCommandBufferManager::SubmitActiveCmdBuffer(Ref<Semaphore> SignalSemaphore)
{
    check(ActiveCmdBufferRef);

    if (!ActiveCmdBufferRef->IsSubmitted() && ActiveCmdBufferRef->HasBegun()) {
        if (!ActiveCmdBufferRef->IsOutsideRenderPass()) {
            LOG(LogVulkanRHI, Warning, "Forcing EndRenderPass() for submission");
            ActiveCmdBufferRef->EndRenderPass();
        }

        ActiveCmdBufferRef->End();

        if (SignalSemaphore) {
            Queue->Submit(ActiveCmdBufferRef, SignalSemaphore->GetHandle());
        } else {
            Queue->Submit(ActiveCmdBufferRef);
        }
    }
    ActiveCmdBufferRef->SetName("Unused.Buffer");
    ActiveCmdBufferRef = nullptr;
}

void VulkanCommandBufferManager::SubmitActiveCmdBufferFromPresent(Ref<Semaphore> SignalSemaphore)
{
    if (SignalSemaphore) {
        Queue->Submit(ActiveCmdBufferRef, SignalSemaphore->GetHandle());
    } else {
        Queue->Submit(ActiveCmdBufferRef);
    }
}

VulkanCmdBuffer* VulkanCommandBufferManager::FindAvailableCmdBuffer()
{
    VulkanCmdBuffer* const NewBuffer = Pool->GetCommandBuffer();
    NewBuffer->Begin();
    return NewBuffer;
}

}    // namespace VulkanRHI
