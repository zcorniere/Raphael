#include "Engine/Core/RHI/RHICommandList.hxx"
#include "Engine/Core/RHI/Resources/RHITexture.hxx"
#include "Engine/Core/RHI/Resources/RHIViewport.hxx"

// Texture
void RHITexture::Resize(const glm::uvec2& Size)
{
    Description.Extent = Size;
    Invalidate();
}

// Viewport
void RHIViewport::BeginDrawViewport()
{
    ENQUEUE_RENDER_COMMAND(BeginDrawViewport)
    ([instance = WeakRef(this)](RHICommandList& CommandList) -> void {
        if (!instance.IsValid()) {
            return;
        }
        CommandList.BeginRenderingViewport(instance);
    });
}

void RHIViewport::EndDrawViewport()
{
    ENQUEUE_RENDER_COMMAND(EndDrawViewport)
    ([instance = WeakRef(this)](RHICommandList& CommandList) -> void {
        if (!instance.IsValid()) {
            return;
        }
        CommandList.EndRenderingViewport(instance, true);
    });
}
