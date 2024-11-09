#pragma once

#include "VulkanRHI/VulkanSynchronization.hxx"

namespace VulkanRHI
{

class FVulkanDevice;
class FVulkanQueue;
class FVulkanCommandContext;
class VulkanCommandBufferPool;
class VulkanCommandBufferManager;
class RVulkanGraphicsPipeline;

class FVulkanCmdBuffer : public FNamedClass, public IDeviceChild
/// This class encapsulate a Vulkan command buffer
{
    RPH_NONCOPYABLE(FVulkanCmdBuffer)
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
    FVulkanCmdBuffer() = delete;
    FVulkanCmdBuffer(FVulkanDevice* InDevice, VulkanCommandBufferPool* InCommandPool);
    ~FVulkanCmdBuffer();

    virtual void SetName(std::string_view InName) override;

    /// Mark the command buffer as ready to be record command
    void Begin();
    /// End the recording of the command buffer
    void End();

    void BeginRendering(const VkRenderingInfo& RenderingInfo);
    void EndRendering();

    /// Adds a pipeline semaphore for the given stage
    void AddWaitSemaphore(VkPipelineStageFlags InWaitFlags, const Ref<RSemaphore>& InSemaphore);

    inline VkCommandBuffer GetHandle() const
    {
        return m_CommandBufferHandle;
    }

    inline RFence* GetFence()
    {
        return m_Fence.Raw();
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

    /// Check the internal fence has been signaled, meaning the command buffer is ready to be reset
    void RefreshFenceStatus();

private:
    void Allocate();
    void Free();

public:
    /// The current state of the command buffer
    EState State;

private:
    VulkanCommandBufferPool* m_OwnerPool = nullptr;
    ;

    Ref<RFence> m_Fence = nullptr;
    TArray<VkPipelineStageFlags> WaitFlags;
    TArray<Ref<RSemaphore>> WaitSemaphore;

    VkCommandBuffer m_CommandBufferHandle = VK_NULL_HANDLE;

    friend class FVulkanQueue;
};

/// This class encapsulate a Vulkan command buffer pool
class VulkanCommandBufferPool : public FNamedClass, public IDeviceChild
{
    RPH_NONCOPYABLE(VulkanCommandBufferPool)
public:
    VulkanCommandBufferPool() = delete;
    VulkanCommandBufferPool(FVulkanDevice* InDevice);
    ~VulkanCommandBufferPool();

    virtual void SetName(std::string_view InName) override final;

    void Initialize(uint32 QueueFamilyIndex);

    /// Find a valid command buffer, or create it if needed
    /// @return A valid command buffer, ready to be used
    [[nodiscard]] FVulkanCmdBuffer* GetCommandBuffer();

    VkCommandPool GetHandle() const
    {
        return m_Handle;
    }

    void RefreshFenceStatus(const FVulkanCmdBuffer* SkipCmdBuffer);

private:
    VkCommandPool m_Handle = VK_NULL_HANDLE;
    TArray<FVulkanCmdBuffer*> m_CmdBuffers;
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
    VulkanCommandBufferManager(FVulkanDevice* InDevice, FVulkanQueue* InQueue);
    /// Destruct the command buffer manager, and all its command buffers as well
    ~VulkanCommandBufferManager();

    /// Update the fences of all cmd buffers except the one givent as argument
    /// @arg SkipCmdBuffer the command buffer to skip
    void RefreshFenceStatus(FVulkanCmdBuffer* SkipCmdBuffer = nullptr)
    {
        Pool->RefreshFenceStatus(SkipCmdBuffer);
    }

    void WaitForCmdBuffer(FVulkanCmdBuffer* CmdBuffer, float TimeInSecondsToWait = 10.0f);

    /// @brief Return the active command buffer
    /// @note Calling this function will submit the upload command buffer to the queue
    FVulkanCmdBuffer* GetActiveCmdBuffer();
    /// @brief Return the upload command buffer
    FVulkanCmdBuffer* GetUploadCmdBuffer();

    /// Ask the manager to find an available command buffer for the next frame
    void PrepareForNewActiveCommandBuffer();

    /// Submit the upload command buffer to the queue
    void SubmitUploadCmdBuffer(const Ref<RSemaphore>& SignalSemaphore = nullptr);

    /// Submit the active command buffer to the queue
    void SubmitActiveCmdBuffer(const Ref<RSemaphore>& SignalSemaphore = nullptr);
    void SubmitActiveCmdBufferFromPresent(const Ref<RSemaphore>& SignalSemaphore = nullptr);

private:
    FVulkanCmdBuffer* FindAvailableCmdBuffer();

private:
    FVulkanQueue* Queue = nullptr;

    VulkanCommandBufferPool* Pool = nullptr;

    FVulkanCmdBuffer* ActiveCmdBufferRef = nullptr;
    FVulkanCmdBuffer* UploadCmdBufferRef = nullptr;
};

}    // namespace VulkanRHI
