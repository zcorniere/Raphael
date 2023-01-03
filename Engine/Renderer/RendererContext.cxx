#include "Engine/Renderer/RendererContext.hxx"

#include "Engine/Renderer/RendererAPI.hxx"

#include "Engine/Renderer/Vulkan/VulkanContext.hxx"

namespace Raphael
{

Ref<RendererContext> RendererContext::Create()
{
    switch (RendererAPI::Current()) {
        case RendererAPIType::Vulkan: return Ref<VulkanContext>::Create();
        default: checkMsg(false, "Unknowned Renderer !"); return nullptr;
    }
    return nullptr;
}
}    // namespace Raphael
