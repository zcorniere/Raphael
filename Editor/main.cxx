#include "Engine/Core/Log.hxx"
#include "Engine/Renderer/Vulkan/VulkanDynamicRHI.hxx"

int main(int ac, char **av)
{
    Raphael::Log::Init();

    Raphael::RHI::VulkanDynamicRHI RHI;
    RHI.Init();
    RHI.Shutdown();

    Raphael::Log::Shutdown();
    return 0;
}
