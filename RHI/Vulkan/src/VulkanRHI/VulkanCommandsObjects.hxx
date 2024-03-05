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

class VulkanCmdBuffer : public NamedClass, public IDeviceChild
/// This class encapsulate a Vulkan command buffer
{
    RPH_NONCOPYABLE(VulkanCmdBuffer)
public:
    /// The state of the command buffer
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
    VulkanCmdBuffer(VulkanDevice* InDevice, VulkanCommandBufferPool* InCommandPool);
    ~VulkanCmdBuffer();

    virtual void SetName(std::string_view InName) override;

    /// Mark the command buffer as ready to be record command
    void Begin();
    /// End the recording of the command buffer
    void End();

    void BeginRenderPass(const VkRenderPassBeginInfo& RenderPassBeginInfo);
    void EndRenderPass();

    /// Adds a pipeline semaphore for the given stage
    void AddWaitSemaphore(VkPipelineStageFlags InWaitFlags, Ref<Semaphore> InSemaphore);

    inline VkCommandBuffer GetHandle() const
    {
        return m_CommandBufferHandle;
    }

    inline VulkanCommandBufferPool* GetOwner() const
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
    /// The current state of the command buffer
    EState State;

private:
    VulkanCommandBufferPool* m_OwnerPool;

    Ref<Fence> m_Fence;
    Array<VkPipelineStageFlags> WaitFlags;
    Array<Ref<Semaphore>> WaitSemaphore;

    VkCommandBuffer m_CommandBufferHandle;

    friend class VulkanCommandBufferManager;
    friend class VulkanCommandBufferPool;
    friend class VulkanQueue;
};

/// This class encapsulate a Vulkan command buffer pool
class VulkanCommandBufferPool : public NamedClass, public IDeviceChild
{
    RPH_NONCOPYABLE(VulkanCommandBufferPool)
public:
    VulkanCommandBufferPool() = delete;
    VulkanCommandBufferPool(VulkanDevice* InDevice, VulkanCommandBufferManager* InManager);
    ~VulkanCommandBufferPool();

    virtual void SetName(std::string_view InName) override final;

    void Initialize(uint32 QueueFamilyIndex);

    /// Allocated the vulkan command pool object
    [[nodiscard]] VulkanCmdBuffer* CreateCmdBuffer();

    VkCommandPool GetHandle() const
    {
        return m_Handle;
    }

    void RefreshFenceStatus(const VulkanCmdBuffer* SkipCmdBuffer);

private:
    VkCommandPool m_Handle;
    Array<VulkanCmdBuffer*> m_CmdBuffers;
    Array<VulkanCmdBuffer*> m_FreeCmdBuffers;

    VulkanCommandBufferManager* p_Manager;

    friend class VulkanCommandBufferManager;
};

/// @brief Manages the command buffers belonging to a queue
/// @details It is responsible for the allocation and submission of command buffers
///
/// It manage two command buffers:
/// - The active command buffer, used for rendering
/// - The upload command buffer, used for uploading resources to the GPU
class VulkanCommandBufferManager : public IDeviceChild
{
    RPH_NONCOPYABLE(VulkanCommandBufferManager)
public:
    VulkanCommandBufferManager() = delete;
    /// Construct a command buffer manager for the given queue
    VulkanCommandBufferManager(VulkanDevice* InDevice, VulkanQueue* InQueue);
    /// Destruct the command buffer manager, and all its command buffers as well
    ~VulkanCommandBufferManager();

    /// Update the fences of all cmd buffers except the one givent as argument
    /// @arg SkipCmdBuffer the command buffer to skip
    void RefreshFenceStatus(VulkanCmdBuffer* SkipCmdBuffer = nullptr)
    {
        Pool->RefreshFenceStatus(SkipCmdBuffer);
    }

    /// @brief Return the active command buffer
    /// @note Calling this function will submit the upload command buffer to the queue
    VulkanCmdBuffer* GetActiveCmdBuffer();
    /// @brief Return the upload command buffer
    VulkanCmdBuffer* GetUploadCmdBuffer();

    /// Ask the manager to find an available command buffer for the next frame
    void PrepareForNewActiveCommandBuffer();

    /// Submit the upload command buffer to the queue
    void SubmitUploadCmdBuffer(Ref<Semaphore> SignalSemaphore = nullptr);

    /// Submit the active command buffer to the queue
    void SubmitActiveCmdBuffer(Ref<Semaphore> SignalSemaphore = nullptr);
    void SubmitActiveCmdBufferFromPresent(Ref<Semaphore> SignalSemaphore = nullptr);

private:
    VulkanCmdBuffer* FindAvailableCmdBuffer();

private:
    VulkanQueue* Queue;

    VulkanCommandBufferPool* Pool;

    VulkanCmdBuffer* ActiveCmdBufferRef;
    VulkanCmdBuffer* UploadCmdBufferRef;
};

}    // namespace VulkanRHI
