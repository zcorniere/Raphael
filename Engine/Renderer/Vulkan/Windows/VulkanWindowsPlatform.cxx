#include "Engine/Renderer/Vulkan/Windows/VulkanWindowsPlatform.hxx"

#include "Engine/Renderer/Vulkan/VulkanLoader.hxx"
#include "Engine/Renderer/Vulkan/VulkanRHI.hxx"
#include "Engine/Renderer/Vulkan/VulkanUtils.hxx"

#include "Engine/Renderer/Window.hxx"

#include <SDL.h>
#include <SDL_vulkan.h>

namespace VulkanRHI
{

#define DEFINE_VK_ENTRYPOINTS(Type, Func) Type VulkanAPI::Func = NULL;

VK_ENTRYPOINT_ALL(DEFINE_VK_ENTRYPOINTS)
VK_ENTRYPOINTS_DEBUG_UTILS(DEFINE_VK_ENTRYPOINTS)

#undef DEFINE_VK_ENTRYPOINTS

void *VulkanWindowsPlatform::VulkanLib = nullptr;
bool VulkanWindowsPlatform::bAttemptedLoad = false;

bool VulkanWindowsPlatform::LoadVulkanLibrary()
{
   return false;
}

bool VulkanWindowsPlatform::LoadVulkanInstanceFunctions(VkInstance inInstance)
{
   return false;
}

void VulkanWindowsPlatform::FreeVulkanLibrary()
{
   
}

void VulkanWindowsPlatform::GetInstanceExtensions([[maybe_unused]] std::vector<const char *> &OutExtensions)
{

}

void VulkanWindowsPlatform::GetDeviceExtensions([[maybe_unused]] VulkanDevice *Device,
                                              [[maybe_unused]] std::vector<const char *> &OutExtensions)
{
}

void VulkanWindowsPlatform::CreateSurface(void *WindowHandle, VkInstance Instance, VkSurfaceKHR *OutSurface)
{

}

}    // namespace VulkanRHI
