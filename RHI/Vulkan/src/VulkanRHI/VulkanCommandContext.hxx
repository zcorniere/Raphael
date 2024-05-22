#pragma once

#include "Engine/Core/RHI/RHIContext.hxx"

namespace VulkanRHI
{

class VulkanDevice;
class VulkanQueue;
class VulkanPendingState;

class VulkanTexture;

class VulkanCommandContext : public RHIContext
{
public:
    VulkanCommandContext(VulkanDevice* InDevice, VulkanQueue* InGraphicsQueue, VulkanQueue* InPresentQueue);
    ~VulkanCommandContext();

public:
    /// @brief Mark the beginning of a new frame
    virtual void BeginFrame() override;
    /// @brief Mark the end of the current frame
    virtual void EndFrame() override;

    /// @brief Indicate the RHI that we are starting drawing in the given viewport
    virtual void RHIBeginDrawingViewport(RHIViewport* const Viewport) override;
    /// @brief Indicate the RHI that we are done drawing in the given viewport
    virtual void RHIEndDrawningViewport(RHIViewport* const Viewport) override;

    virtual void RHIBeginRendering(const RHIRenderPassDescription& Description) override;
    virtual void RHIEndRendering() override;

    virtual void SetPipeline(Ref<RHIGraphicsPipeline>& Pipeline) override;

    virtual void SetVertexBuffer(Ref<RHIBuffer>& VertexBuffer, uint32 BufferIndex, uint32 Offset) override;

    virtual void SetViewport(glm::vec3 Min, glm::vec3 Max) override;
    virtual void SetScissor(glm::ivec2 Offset, glm::uvec2 Size) override;

    virtual void Draw(uint32 BaseVertexIndex, uint32 NumPrimitives, uint32 NumInstances) override;

    /// @brief VulkanRHI only, set the layout of the given texture
    void SetLayout(VulkanTexture* const Texture, VkImageLayout Layout);

private:
    std::unique_ptr<VulkanPendingState> PendingState;

    VulkanDevice* const Device = nullptr;
    VulkanQueue* const GfxQueue = nullptr;
    VulkanQueue* const PresentQueue = nullptr;
};

}    // namespace VulkanRHI
