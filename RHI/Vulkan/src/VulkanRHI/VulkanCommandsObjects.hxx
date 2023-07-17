#pragma once

#include "VulkanRHI/VulkanLoader.hxx"

#include "VulkanRHI/VulkanSynchronization.hxx"

namespace VulkanRHI
{

class VulkanDevice;
class VulkanQueue;
class VulkanCommandContext;
class VulkanCommandBufferPool;
class VulkanCommandBufferManager;

class VulkanCmdBuffer : public RObject
{
public:
    enum class EState : uint8 {
        ReadyForBegin = 0,
        IsInsideBegin = 1,
        IsInsideRenderPass = 2,
        HasEnded = 3,
        Submitted = 4,
        NotAllocated = 5,
        NeedReset = 6,
    };

public:
    VulkanCmdBuffer() = delete;
    VulkanCmdBuffer(Ref<VulkanDevice> InDevice, WeakRef<VulkanCommandBufferPool> InCommandPool);
    ~VulkanCmdBuffer();

    virtual void SetName(std::string_view InName) override;

    void Begin();
    void End();

    void BeginRenderPass(const VkRenderPassBeginInfo& RenderPassBeginInfo);
    void EndRenderPass();

    void AddWaitSemaphore(VkPipelineStageFlags InWaitFlags, Ref<Semaphore>& InSemaphore);

    inline VkCommandBuffer GetHandle() const
    {
        return m_CommandBufferHandle;
    }

    inline WeakRef<VulkanCommandBufferPool> GetOwner() const
    {
        return m_OwnerPool;
    }

    inline bool IsInsideRenderPass() const
    {
        return State == EState::IsInsideRenderPass;
    }

    inline bool IsOutsideRenderPass() const
    {
        return State == EState::IsInsideBegin;
    }

    inline bool HasBegun() const
    {
        return State == EState::IsInsideBegin || State == EState::IsInsideRenderPass;
    }

    inline bool HasEnded() const
    {
        return State == EState::HasEnded;
    }

    inline bool IsSubmitted() const
    {
        return State == EState::Submitted;
    }

    inline bool IsAllocated() const
    {
        return State != EState::NotAllocated;
    }

private:
    void Allocate();
    void Free();

    void RefreshFenceStatus();

public:
    EState State;

private:
    Ref<VulkanDevice> m_Device;
    WeakRef<VulkanCommandBufferPool> m_OwnerPool;

    Ref<Fence> m_Fence;
    Array<VkPipelineStageFlags> WaitFlags;
    Array<Ref<Semaphore>> WaitSemaphore;

    VkCommandBuffer m_CommandBufferHandle;

    friend class VulkanCommandBufferManager;
    friend class VulkanCommandBufferPool;
    friend class VulkanQueue;
};

class VulkanCommandBufferPool : public RObject
{
public:
    VulkanCommandBufferPool() = delete;
    VulkanCommandBufferPool(Ref<VulkanDevice> InDevice, VulkanCommandBufferManager* InManager);
    ~VulkanCommandBufferPool();

    void Initialize(uint32 QueueFamilyIndex);
    [[nodiscard]] Ref<VulkanCmdBuffer> CreateCmdBuffer();

    VkCommandPool GetHandle() const
    {
        return m_Handle;
    }

    void RefreshFenceStatus(const Ref<VulkanCmdBuffer>& SkipCmdBuffer);

private:
    VkCommandPool m_Handle;
    Array<Ref<VulkanCmdBuffer>> m_CmdBuffers;
    Array<Ref<VulkanCmdBuffer>> m_FreeCmdBuffers;

    Ref<VulkanDevice> m_Device;
    VulkanCommandBufferManager* p_Manager;

    friend class VulkanCommandBufferManager;
};

class VulkanCommandBufferManager
{
public:
    VulkanCommandBufferManager(Ref<VulkanDevice> InDevice, Ref<VulkanQueue> InQueue);
    ~VulkanCommandBufferManager();

    void Init();
    void Shutdown();

    // Update the fences of all cmd buffers except SkipCmdBuffer
    void RefreshFenceStatus(Ref<VulkanCmdBuffer> SkipCmdBuffer = nullptr)
    {
        Pool->RefreshFenceStatus(SkipCmdBuffer);
    }

    Ref<VulkanCmdBuffer>& GetActiveCmdBuffer();
    Ref<VulkanCmdBuffer>& GetUploadCmdBuffer();

    void PrepareForNewActiveCommandBuffer();

    void SubmitUploadCmdBuffer(Ref<Semaphore> SignalSemaphore = nullptr);

    void SubmitActiveCmdBuffer(Ref<Semaphore> SignalSemaphore = nullptr);
    void SubmitActiveCmdBufferFormPresent(Ref<Semaphore> SignalSemaphore = nullptr);

private:
    Ref<VulkanCmdBuffer> FindAvailableCmdBuffer();

private:
    Ref<VulkanDevice> Device;
    Ref<VulkanQueue> Queue;

    Ref<VulkanCommandBufferPool> Pool;
    Ref<VulkanCmdBuffer> ActiveCmdBuffer;
    Ref<VulkanCmdBuffer> UploadCmdBuffer;
};

}    // namespace VulkanRHI
