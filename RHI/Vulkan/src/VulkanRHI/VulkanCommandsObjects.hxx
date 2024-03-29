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
class VulkanGraphicsPipeline;

class VulkanCmdBuffer : public RObject, public IDeviceChild
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
    VulkanCmdBuffer(VulkanDevice* InDevice, WeakRef<VulkanCommandBufferPool> InCommandPool);
    ~VulkanCmdBuffer();

    virtual void SetName(std::string_view InName) override;

    void Begin();
    void End();

    void BeginRenderPass(const VkRenderPassBeginInfo& RenderPassBeginInfo);
    void EndRenderPass();

    void AddWaitSemaphore(VkPipelineStageFlags InWaitFlags, Ref<Semaphore> InSemaphore);

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
    WeakRef<VulkanCommandBufferPool> m_OwnerPool;

    Ref<Fence> m_Fence;
    Array<VkPipelineStageFlags> WaitFlags;
    Array<Ref<Semaphore>> WaitSemaphore;

    VkCommandBuffer m_CommandBufferHandle;

    friend class VulkanCommandBufferManager;
    friend class VulkanCommandBufferPool;
    friend class VulkanQueue;
};

class VulkanCommandBufferPool : public RObject, public IDeviceChild
{
public:
    VulkanCommandBufferPool() = delete;
    VulkanCommandBufferPool(VulkanDevice* InDevice, VulkanCommandBufferManager* InManager);
    ~VulkanCommandBufferPool();

    virtual void SetName(std::string_view InName) override final;

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

    VulkanCommandBufferManager* p_Manager;

    friend class VulkanCommandBufferManager;
};

class VulkanCommandBufferManager : public IDeviceChild
{
public:
    VulkanCommandBufferManager(VulkanDevice* InDevice, VulkanQueue* InQueue);
    ~VulkanCommandBufferManager();

    // Update the fences of all cmd buffers except SkipCmdBuffer
    void RefreshFenceStatus(Ref<VulkanCmdBuffer> SkipCmdBuffer = nullptr)
    {
        Pool->RefreshFenceStatus(SkipCmdBuffer);
    }

    WeakRef<VulkanCmdBuffer> GetActiveCmdBuffer();
    WeakRef<VulkanCmdBuffer> GetUploadCmdBuffer();

    void PrepareForNewActiveCommandBuffer();

    void SubmitUploadCmdBuffer(Ref<Semaphore> SignalSemaphore = nullptr);

    void SubmitActiveCmdBuffer(Ref<Semaphore> SignalSemaphore = nullptr);
    void SubmitActiveCmdBufferFromPresent(Ref<Semaphore> SignalSemaphore = nullptr);

private:
    Ref<VulkanCmdBuffer> FindAvailableCmdBuffer();

private:
    VulkanQueue* Queue;

    Ref<VulkanCommandBufferPool> Pool;
    Ref<VulkanCmdBuffer> ActiveCmdBuffer;
    Ref<VulkanCmdBuffer> UploadCmdBuffer;
};

}    // namespace VulkanRHI
