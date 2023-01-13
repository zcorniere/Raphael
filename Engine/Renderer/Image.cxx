#include "Engine/Renderer/Image.hxx"

#include "Engine/Renderer/RendererAPI.hxx"

#include "Engine/Renderer/Vulkan/VulkanImage.hxx"

namespace Raphael
{

Ref<Image2D> Image2D::Create(ImageSpecification specification)
{
    switch (RendererAPI::Current()) {
        case RendererAPIType::None: return nullptr;
        case RendererAPIType::Vulkan: return Ref<VulkanImage2D>::Create(specification);
    }
    checkNoEntry();
    return nullptr;
}

}    // namespace Raphael
