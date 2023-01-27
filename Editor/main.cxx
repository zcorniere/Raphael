#include "Engine/Core/Log.hxx"
#include "Engine/Renderer/Vulkan/VulkanDynamicRHI.hxx"

using namespace Raphael;

int main(int ac, char **av)
{
    Raphael::Log::Init();

    Ref<RHI::VulkanDynamicRHI> RHI = Ref<RHI::VulkanDynamicRHI>::Create();
    RHI->Init();
    RHI->Shutdown();

    Raphael::Log::Shutdown();
    return 0;
}
