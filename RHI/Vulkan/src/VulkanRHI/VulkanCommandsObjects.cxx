#include "VulkanRHI/VulkanCommandsObjects.hxx"

#include "VulkanRHI/RenderPass/VulkanRenderPass.hxx"
#include "VulkanRHI/VulkanDevice.hxx"
#include "VulkanRHI/VulkanQueue.hxx"

#include "VulkanRHI/Resources/VulkanGraphicsPipeline.hxx"

namespace VulkanRHI
{

/// ------------------- VulkanCmdBuffer -------------------
VulkanCmdBuffer::VulkanCmdBuffer(VulkanDevice* InDevice, WeakRef<VulkanCommandBufferPool> InCommandPool)
    : State(EState::NotAllocated), m_Device(InDevice), m_OwnerPool(InCommandPool)
{
    Allocate();

    m_Fence = Ref<Fence>::Create(m_Device, false);
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
    RObject::SetName(InName);
    if (m_CommandBufferHandle) {
        VULKAN_SET_DEBUG_NAME(m_Device, VK_OBJECT_TYPE_COMMAND_BUFFER, m_CommandBufferHandle, "{:s}", InName);
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
        VulkanAPI::vkAllocateCommandBuffers(m_Device->GetHandle(), &CreateCmdBufInfo, &m_CommandBufferHandle));
    State = EState::ReadyForBegin;
}

void VulkanCmdBuffer::Free()
{
    check(State != EState::NotAllocated);
    check(m_CommandBufferHandle != VK_NULL_HANDLE);

    VulkanAPI::vkFreeCommandBuffers(m_Device->GetHandle(), m_OwnerPool->GetHandle(), 1, &m_CommandBufferHandle);

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

VulkanCommandBufferPool::VulkanCommandBufferPool(VulkanDevice* InDevice, VulkanCommandBufferManager* InManager)
    : m_Handle(VK_NULL_HANDLE), m_Device(InDevice), p_Manager(InManager)
{
}

VulkanCommandBufferPool::~VulkanCommandBufferPool()
{
    m_CmdBuffers.Clear();
    m_FreeCmdBuffers.Clear();

    VulkanAPI::vkDestroyCommandPool(m_Device->GetHandle(), m_Handle, VULKAN_CPU_ALLOCATOR);
    m_Handle = VK_NULL_HANDLE;
}

void VulkanCommandBufferPool::SetName(std::string_view InName)
{
    RObject::SetName(InName);
    if (m_Handle) {
        VULKAN_SET_DEBUG_NAME(m_Device, VK_OBJECT_TYPE_COMMAND_POOL, m_Handle, "{:s}", InName);
    }
}

void VulkanCommandBufferPool::Initialize(uint32 QueueFamilyIndex)
{
    VkCommandPoolCreateInfo CmdPoolInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = QueueFamilyIndex,
    };
    VK_CHECK_RESULT(
        VulkanAPI::vkCreateCommandPool(m_Device->GetHandle(), &CmdPoolInfo, VULKAN_CPU_ALLOCATOR, &m_Handle));
}

Ref<VulkanCmdBuffer> VulkanCommandBufferPool::CreateCmdBuffer()
{
    // Find already allocated buffer ...
    for (int32 i = m_FreeCmdBuffers.Size() - 1; i >= 0; --i) {
        Ref<VulkanCmdBuffer> CmdBuffer = m_FreeCmdBuffers[i];
        m_FreeCmdBuffers.Pop();
        // ... grab some memory ...
        CmdBuffer->Allocate();
        // put it in the "in use" buffers
        m_CmdBuffers.Add(CmdBuffer);
        return CmdBuffer;
    }

    // No buffer are available, create a new one. It already has memory
    Ref<VulkanCmdBuffer> NewCmdBuffer = Ref<VulkanCmdBuffer>::Create(m_Device, this);
    m_CmdBuffers.Add(NewCmdBuffer);
    return NewCmdBuffer;
}

void VulkanCommandBufferPool::RefreshFenceStatus(const Ref<VulkanCmdBuffer>& SkipCmdBuffer)
{
    for (Ref<VulkanCmdBuffer>& CmdBuffer: m_CmdBuffers) {
        if (CmdBuffer == SkipCmdBuffer)
            continue;
        CmdBuffer->RefreshFenceStatus();
    }
}

/// ------------------- VulkanCommandBufferManager -------------------

VulkanCommandBufferManager::VulkanCommandBufferManager(VulkanDevice* InDevice, VulkanQueue* InQueue)
    : Device(InDevice), Queue(InQueue)
{
    Pool = Ref<VulkanCommandBufferPool>::Create(Device, this);
    Pool->Initialize(Queue->GetFamilyIndex());
    Pool->SetName("Main.CommandPool");

    ActiveCmdBuffer = Pool->CreateCmdBuffer();
    ActiveCmdBuffer->SetName(std::format("Active{:d}.CommandBuffer", GFrameCounter));
    UploadCmdBuffer = nullptr;
}

VulkanCommandBufferManager ::~VulkanCommandBufferManager()
{
}

WeakRef<VulkanCmdBuffer> VulkanCommandBufferManager::GetActiveCmdBuffer()
{
    if (UploadCmdBuffer) {
        SubmitUploadCmdBuffer();
    }
    return ActiveCmdBuffer;
}
WeakRef<VulkanCmdBuffer> VulkanCommandBufferManager::GetUploadCmdBuffer()
{
    if (!UploadCmdBuffer) {
        UploadCmdBuffer = FindAvailableCmdBuffer();
        UploadCmdBuffer->SetName("Upload.CommandBuffer");
    }
    return UploadCmdBuffer;
}

void VulkanCommandBufferManager::PrepareForNewActiveCommandBuffer()
{
    ActiveCmdBuffer = FindAvailableCmdBuffer();
    ActiveCmdBuffer->SetName(std::format("Active{:d}.CommandBuffer", GFrameCounter));
}

void VulkanCommandBufferManager::SubmitUploadCmdBuffer(Ref<Semaphore> SignalSemaphore)
{
    check(UploadCmdBuffer);

    if (!UploadCmdBuffer->IsSubmitted() && UploadCmdBuffer->HasBegun()) {
        check(UploadCmdBuffer->IsOutsideRenderPass());

        UploadCmdBuffer->End();

        if (SignalSemaphore) {
            Queue->Submit(UploadCmdBuffer.Raw(), SignalSemaphore->GetHandle());
        } else {
            Queue->Submit(UploadCmdBuffer.Raw());
        }
    }
    UploadCmdBuffer->SetName("Unused.Buffer");
    UploadCmdBuffer = nullptr;
}

void VulkanCommandBufferManager::SubmitActiveCmdBuffer(Ref<Semaphore> SignalSemaphore)
{
    check(ActiveCmdBuffer);

    if (!ActiveCmdBuffer->IsSubmitted() && ActiveCmdBuffer->HasBegun()) {
        if (!ActiveCmdBuffer->IsOutsideRenderPass()) {
            LOG(LogVulkanRHI, Warning, "Forcing EndRenderPass() for submission");
            ActiveCmdBuffer->EndRenderPass();
        }

        ActiveCmdBuffer->End();

        if (SignalSemaphore) {
            Queue->Submit(ActiveCmdBuffer.Raw(), SignalSemaphore->GetHandle());
        } else {
            Queue->Submit(ActiveCmdBuffer.Raw());
        }
    }
    ActiveCmdBuffer->SetName("Unused.Buffer");
    ActiveCmdBuffer = nullptr;
}

void VulkanCommandBufferManager::SubmitActiveCmdBufferFromPresent(Ref<Semaphore> SignalSemaphore)
{
    if (SignalSemaphore) {
        Queue->Submit(ActiveCmdBuffer.Raw(), SignalSemaphore->GetHandle());
    } else {
        Queue->Submit(ActiveCmdBuffer.Raw());
    }
}

Ref<VulkanCmdBuffer> VulkanCommandBufferManager::FindAvailableCmdBuffer()
{
    for (uint32 Index = 0; Index < Pool->m_CmdBuffers.Size(); Index++) {
        Ref<VulkanCmdBuffer>& CmdBuffer = Pool->m_CmdBuffers[Index];
        CmdBuffer->RefreshFenceStatus();

        if (CmdBuffer->State == VulkanCmdBuffer::EState::ReadyForBegin ||
            CmdBuffer->State == VulkanCmdBuffer::EState::NeedReset) {
            CmdBuffer->Begin();
            return CmdBuffer;
        }
    }

    Ref<VulkanCmdBuffer> NewBuffer = Pool->CreateCmdBuffer();
    NewBuffer->Begin();
    return NewBuffer;
}

}    // namespace VulkanRHI
