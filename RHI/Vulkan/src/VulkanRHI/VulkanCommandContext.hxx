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
    /// @brief Resize the given viewport
    virtual void RHIResizeViewport(RHIViewport* const Viewport, uint32_t Width, uint32_t Height) override;

    /// @brief VulkanRHI only, set the layout of the given texture
    void SetLayout(VulkanTexture* const Texture, VkImageLayout Layout);

private:
    VulkanDevice* const Device = nullptr;
    VulkanQueue* const GfxQueue = nullptr;
    VulkanQueue* const PresentQueue = nullptr;
};

}    // namespace VulkanRHI
