#include "VulkanRHI/VulkanCommandsObjects.hxx"

#include "VulkanRHI/VulkanDevice.hxx"
#include "VulkanRHI/VulkanQueue.hxx"

namespace VulkanRHI
{

/// ------------------- VulkanCmdBuffer -------------------
VulkanCmdBuffer::VulkanCmdBuffer(Ref<VulkanDevice> InDevice, WeakRef<VulkanCommandBufferPool> InCommandPool)
    : State(EState::NotAllocated), m_Device(InDevice), m_OwnerPool(InCommandPool)
{
    Allocate();

    m_Fence = Ref<Fence>::Create(m_Device, false);
}

VulkanCmdBuffer::~VulkanCmdBuffer()
{
    if (State == EState::Submitted) {
        LOG(LogVulkanRHI, Warn, "Attempting to destroy a buffer still in flight ! Waiting 16ms so it can be destroyed");
        // Wait 16 ms
        m_Fence->Wait(16 * 1000 * 1000LL);
        m_Fence->Reset();
    }

    if (State != EState::NotAllocated) { Free(); }
}

void VulkanCmdBuffer::Begin()
{
    if (State == EState::NeedReset) {
        VulkanAPI::vkResetCommandBuffer(m_CommandBufferHandle, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
    } else {
        checkMsg(State == EState::ReadyForBegin, "Can't Begin as we're NOT ready! CmdBuffer {:p} State={:d}",
                 (void *)m_CommandBufferHandle, (int32)State);
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
    checkMsg(IsOutsideRenderPass(), "Can't End as we're inside a render pass! CmdBuffer {:p} State={:d}",
             (void *)m_CommandBufferHandle, (int32)State);

    VK_CHECK_RESULT(VulkanAPI::vkEndCommandBuffer(m_CommandBufferHandle));
    State = EState::HasEnded;
}

void VulkanCmdBuffer::BeginRenderPass(/* Argument */) { checkNoEntry(); }
void VulkanCmdBuffer::EndRenderPass()
{
    checkMsg(IsInsideRenderPass(), "Can't EndRenderPass as we're NOT inside one! CmdBuffer {:p} State={:d}",
             (void *)m_CommandBufferHandle, (int32)State);
    VulkanAPI::vkCmdEndRenderPass(m_CommandBufferHandle);
    State = EState::IsInsideBegin;
}

void VulkanCmdBuffer::AddWaitSemaphore(VkPipelineStageFlags InWaitFlags, Ref<Semaphore> &InSemaphore)
{
    // TODO: check if InSemaphore is not already inside WaitSemaphore (std::vector does not have find() / contains())
    WaitFlags.push_back(InWaitFlags);
    WaitSemaphore.push_back(InSemaphore);
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
        VulkanAPI::vkAllocateCommandBuffers(m_Device->GetInstanceHandle(), &CreateCmdBufInfo, &m_CommandBufferHandle));
    State = EState::ReadyForBegin;
}

void VulkanCmdBuffer::Free()
{
    check(State != EState::NotAllocated);
    check(m_CommandBufferHandle != VK_NULL_HANDLE);

    VulkanAPI::vkFreeCommandBuffers(m_Device->GetInstanceHandle(), m_OwnerPool->GetHandle(), 1, &m_CommandBufferHandle);

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
        WaitSemaphore.clear();

        m_Fence->Reset();
        State = EState::NeedReset;
    }
}

/// ------------------- VulkanCommandBufferPool -------------------

VulkanCommandBufferPool::VulkanCommandBufferPool(Ref<VulkanDevice> InDevice, VulkanCommandBufferManager *InManager)
    : m_Handle(VK_NULL_HANDLE), m_Device(InDevice), p_Manager(InManager)
{
}

VulkanCommandBufferPool ::~VulkanCommandBufferPool()
{
    m_CmdBuffers.clear();
    m_FreeCmdBuffers.clear();

    VulkanAPI::vkDestroyCommandPool(m_Device->GetInstanceHandle(), m_Handle, nullptr);
    m_Handle = VK_NULL_HANDLE;
}

void VulkanCommandBufferPool::Initialize(uint32 QueueFamilyIndex)
{
    VkCommandPoolCreateInfo CmdPoolInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = QueueFamilyIndex,
    };
    VK_CHECK_RESULT(VulkanAPI::vkCreateCommandPool(m_Device->GetInstanceHandle(), &CmdPoolInfo, nullptr, &m_Handle));
}

Ref<VulkanCmdBuffer> VulkanCommandBufferPool::CreateCmdBuffer()
{
    // Find already allocated buffer ...
    for (int32 i = m_FreeCmdBuffers.size() - 1; i >= 0; --i) {
        Ref<VulkanCmdBuffer> CmdBuffer = m_FreeCmdBuffers[i];

        m_FreeCmdBuffers.pop_back();
        // ... grab some memory ...
        CmdBuffer->Allocate();

        // put it in the "in use" buffers
        m_CmdBuffers.push_back(CmdBuffer);

        return CmdBuffer;
    }

    // No buffer are available, create a new one. It already has memory
    Ref<VulkanCmdBuffer> NewCmdBuffer = Ref<VulkanCmdBuffer>::Create(m_Device, this);
    m_CmdBuffers.push_back(NewCmdBuffer);
    return NewCmdBuffer;
}

void VulkanCommandBufferPool::RefreshFenceStatus(Ref<VulkanCmdBuffer> &SkipCmdBuffer)
{
    for (Ref<VulkanCmdBuffer> &CmdBuffer: m_CmdBuffers) {
        if (CmdBuffer == SkipCmdBuffer) continue;
        CmdBuffer->RefreshFenceStatus();
    }
}

/// ------------------- VulkanCommandBufferManager -------------------

VulkanCommandBufferManager::VulkanCommandBufferManager(Ref<VulkanDevice> InDevice, Ref<VulkanQueue> InQueue)
    : Device(InDevice), Queue(InQueue)
{
    Pool = Ref<VulkanCommandBufferPool>::Create(Device, this);
    Pool->Initialize(Queue->GetFamilyIndex());

    ActiveCmdBuffer = Pool->CreateCmdBuffer();
    UploadCmdBuffer = nullptr;
}
VulkanCommandBufferManager ::~VulkanCommandBufferManager() {}

void VulkanCommandBufferManager::Init() { ActiveCmdBuffer->Begin(); }
void VulkanCommandBufferManager::Shutdown() {}

Ref<VulkanCmdBuffer> &VulkanCommandBufferManager::GetActiveCmdBuffer()
{
    if (UploadCmdBuffer) { SubmitUploadCmdBuffer(); }
    return ActiveCmdBuffer;
}
Ref<VulkanCmdBuffer> &VulkanCommandBufferManager::GetUploadCmdBuffer()
{
    if (!UploadCmdBuffer) { UploadCmdBuffer = FindAvailableCmdBuffer(); }
    return UploadCmdBuffer;
}

void VulkanCommandBufferManager::PrepareForNewActiveCommandBuffer() { ActiveCmdBuffer = FindAvailableCmdBuffer(); }

void VulkanCommandBufferManager::SubmitUploadCmdBuffer(Ref<Semaphore> SignalSemaphore)
{
    check(UploadCmdBuffer);

    if (!UploadCmdBuffer->IsSubmitted() && UploadCmdBuffer->HasBegun()) {
        check(UploadCmdBuffer->IsOutsideRenderPass());

        UploadCmdBuffer->End();

        if (SignalSemaphore) {
            Queue->Submit(UploadCmdBuffer, SignalSemaphore->GetHandle());
        } else {
            Queue->Submit(UploadCmdBuffer);
        }
    }
    UploadCmdBuffer = nullptr;
}

void VulkanCommandBufferManager::SubmitActiveCmdBuffer(Ref<Semaphore> SignalSemaphore)
{
    check(ActiveCmdBuffer);

    if (!ActiveCmdBuffer->IsSubmitted() && ActiveCmdBuffer->HasBegun()) {
        if (!ActiveCmdBuffer->IsOutsideRenderPass()) {
            LOG(LogVulkanRHI, Warn, "Forcing EndRenderPass() for submission");
            ActiveCmdBuffer->EndRenderPass();
        }

        ActiveCmdBuffer->End();

        if (SignalSemaphore) {
            Queue->Submit(ActiveCmdBuffer, SignalSemaphore->GetHandle());
        } else {
            Queue->Submit(ActiveCmdBuffer);
        }
    }
    ActiveCmdBuffer = nullptr;
}

void VulkanCommandBufferManager::SubmitActiveCmdBufferFormPresent(Ref<Semaphore> SignalSemaphore)
{
    if (SignalSemaphore) {
        Queue->Submit(ActiveCmdBuffer, SignalSemaphore->GetHandle());
    } else {
        Queue->Submit(ActiveCmdBuffer);
    }
}

Ref<VulkanCmdBuffer> VulkanCommandBufferManager::FindAvailableCmdBuffer()
{
    for (uint32 Index = 0; Index < Pool->m_CmdBuffers.size(); Index++) {
        Ref<VulkanCmdBuffer> &CmdBuffer = Pool->m_CmdBuffers[Index];
        CmdBuffer->RefreshFenceStatus();

        if (CmdBuffer->State == VulkanCmdBuffer::EState::ReadyForBegin ||
            CmdBuffer->State == VulkanCmdBuffer::EState::NeedReset) {
            CmdBuffer->Begin();
            return CmdBuffer;
        } else {
            check(CmdBuffer->State == VulkanCmdBuffer::EState::Submitted);
        }
    }

    Ref<VulkanCmdBuffer> NewBuffer = Pool->CreateCmdBuffer();
    NewBuffer->Begin();
    return NewBuffer;
}

}    // namespace VulkanRHI
