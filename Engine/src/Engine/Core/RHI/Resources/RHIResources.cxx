#include "Engine/Core/RHI/RHICommandQueue.hxx"
#include "Engine/Core/RHI/Resources/RHIShader.hxx"
#include "Engine/Core/RHI/Resources/RHITexture.hxx"
#include "Engine/Core/RHI/Resources/RHIViewport.hxx"

#include "Engine/Core/RHI/RHI.hxx"

void RHIViewport::BeginDrawViewport()
{
    ENQUEUE_RENDER_COMMAND(BeginDrawViewport)([instance = Ref(this)]() mutable { instance->RT_BeginDrawViewport(); });
}

void RHIViewport::EndDrawViewport()
{
    ENQUEUE_RENDER_COMMAND(EndDrawViewport)([instance = Ref(this)]() mutable { instance->RT_EndDrawViewport(); });
}

void RHIViewport::ResizeViewport(uint32 Width, uint32 Height)
{
    ENQUEUE_RENDER_COMMAND(ResizeViewport)
    ([instance = Ref(this), Width, Height]() mutable { instance->RT_ResizeViewport(Width, Height); });
}
