#pragma once

#include "Engine/Core/RHI/RHIContext.hxx"

namespace VulkanRHI
{

class VulkanDevice;
class VulkanQueue;

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

    virtual void TmpDraw(Ref<RHIGraphicsPipeline>& Pipeline) override;

    /// @brief VulkanRHI only, set the layout of the given texture
    void SetLayout(VulkanTexture* const Texture, VkImageLayout Layout);

private:
    VulkanDevice* const Device = nullptr;
    VulkanQueue* const GfxQueue = nullptr;
    VulkanQueue* const PresentQueue = nullptr;
};

}    // namespace VulkanRHI
