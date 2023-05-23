#include "VulkanRHI/VulkanPlatform.hxx"
#include "VulkanRHI/VulkanDevice.hxx"
#include "VulkanRHI/VulkanLoader.hxx"
#include "VulkanRHI/VulkanRHI.hxx"
#include "VulkanRHI/VulkanUtils.hxx"

#include "Engine/Core/Window.hxx"
#include "Engine/Platforms/Platform.hxx"
#include "Engine/Platforms/PlatformMisc.hxx"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

namespace VulkanRHI
{

#define DEFINE_VK_ENTRYPOINTS(Type, Func) Type VulkanAPI::Func = nullptr;

VK_ENTRYPOINT_ALL(DEFINE_VK_ENTRYPOINTS)
VK_ENTRYPOINTS_DEBUG_UTILS(DEFINE_VK_ENTRYPOINTS)

#undef DEFINE_VK_ENTRYPOINTS

static Ref<IExternalModule> s_VulkanModuleHandle = nullptr;

#if defined(PLATFORM_WINDOWS)
static constexpr auto VulkanLibraryName = "vulkan-1.dll";
#elif defined(PLATFORM_LINUX)
static constexpr auto VulkanLibraryName = "libvulkan.so.1";
#else
    #error "Unsupported Platform"
#endif

bool VulkanPlatform::LoadVulkanLibrary()
{
    if (s_VulkanModuleHandle) {
        return true;
    }

    s_VulkanModuleHandle = PlatformMisc::LoadExternalModule(VulkanLibraryName);

    if (s_VulkanModuleHandle == nullptr) {
        return false;
    }

    bool bFoundAllEntryPoints = true;
#define CHECK_VK_ENTRYPOINTS(Type, Func)                                   \
    if (VulkanAPI::Func == nullptr) {                                      \
        bFoundAllEntryPoints = false;                                      \
        LOG(LogVulkanRHI, Error, "Failed to find entry point for " #Func); \
    }

#define GET_VK_ENTRYPOINTS(Type, Func) VulkanAPI::Func = s_VulkanModuleHandle->GetSymbol<Type>(#Func);

    VK_ENTRYPOINTS_BASE(GET_VK_ENTRYPOINTS);
    VK_ENTRYPOINTS_BASE(CHECK_VK_ENTRYPOINTS);

    if (!bFoundAllEntryPoints) {
        FreeVulkanLibrary();
        return false;
    }

#undef CHECK_VK_ENTRYPOINTS
#undef GET_VK_ENTRYPOINTS
    return true;
}

bool VulkanPlatform::LoadVulkanInstanceFunctions(VkInstance inInstance)
{
    bool bFoundAllEntryPoints = true;
#define CHECK_VK_ENTRYPOINTS(Type, Func)                                     \
    if (VulkanAPI::Func == nullptr) {                                        \
        bFoundAllEntryPoints = false;                                        \
        LOG(LogVulkanRHI, Warning, "Failed to find entry point for " #Func); \
    }

#define GETINSTANCE_VK_ENTRYPOINTS(Type, Func) \
    VulkanAPI::Func = (Type)VulkanAPI::vkGetInstanceProcAddr(inInstance, #Func);

    VK_ENTRYPOINTS_INSTANCE(GETINSTANCE_VK_ENTRYPOINTS);
    VK_ENTRYPOINTS_INSTANCE(CHECK_VK_ENTRYPOINTS);
    VK_ENTRYPOINTS_SURFACE_INSTANCE(GETINSTANCE_VK_ENTRYPOINTS);
    VK_ENTRYPOINTS_SURFACE_INSTANCE(CHECK_VK_ENTRYPOINTS);

    if (!bFoundAllEntryPoints) {
        return false;
    }

    VK_ENTRYPOINTS_DEBUG_UTILS(GETINSTANCE_VK_ENTRYPOINTS);

#undef GETINSTANCE_VK_ENTRYPOINTS
#undef CHECK_VK_ENTRY_POINTS
    return true;
}

void VulkanPlatform::FreeVulkanLibrary()
{
    if (s_VulkanModuleHandle != nullptr) {
#define CLEAR_VK_ENTRYPOINTS(Type, Func) VulkanAPI::Func = nullptr;
        VK_ENTRYPOINT_ALL(CLEAR_VK_ENTRYPOINTS);
#undef CLEAR_VK_ENTRYPOINTS

        s_VulkanModuleHandle = nullptr;
    }
}

void VulkanPlatform::GetInstanceExtensions(std::vector<const char*>& OutExtensions)
{
    Window::EnsureSDLInit();

#if defined(PLATFORM_WINDOWS)
    OutExtensions.push_back("VK_KHR_surface");
    OutExtensions.push_back("VK_KHR_win32_surface");
#elif defined(PLATFORM_LINUX)
    const char* SDLDriver = SDL_GetCurrentVideoDriver();
    check(SDLDriver);

    if (strcmp(SDLDriver, "x11") == 0) {
        OutExtensions.push_back("VK_KHR_xlib_surface");
    } else if (strcmp(SDLDriver, "wayland") == 0) {
        OutExtensions.push_back("VK_KHR_wayland_surface");
    } else {
        checkNoEntry();
    }
#else
    #error "Unsupported Platform"
#endif
}

void VulkanPlatform::GetDeviceExtensions([[maybe_unused]] Ref<VulkanDevice> Device,
                                         [[maybe_unused]] std::vector<const char*>& OutExtensions)
{
}

void VulkanPlatform::CreateSurface(void* WindowHandle, VkInstance Instance, VkSurfaceKHR* OutSurface)
{
    Window::EnsureSDLInit();

    if (SDL_Vulkan_CreateSurface((SDL_Window*)WindowHandle, Instance, OutSurface) == SDL_FALSE) {
        LOG(LogVulkanRHI, Fatal, "Error initializing SDL Vulkan Surface: {}", SDL_GetError());
        checkNoEntry();
    }
}

VkResult VulkanPlatform::Present(VkQueue Queue, VkPresentInfoKHR& PresentInfo)
{
    return VulkanAPI::vkQueuePresentKHR(Queue, &PresentInfo);
}

VkResult VulkanPlatform::CreateSwapchainKHR(VkDevice Device, const VkSwapchainCreateInfoKHR* CreateInfo,
                                            const VkAllocationCallbacks* Allocator, VkSwapchainKHR* Swapchain)
{
    return VulkanAPI::vkCreateSwapchainKHR(Device, CreateInfo, Allocator, Swapchain);
}

void VulkanPlatform::DestroySwapchainKHR(VkDevice Device, VkSwapchainKHR Swapchain,
                                         const VkAllocationCallbacks* Allocator)
{
    VulkanAPI::vkDestroySwapchainKHR(Device, Swapchain, Allocator);
}

}    // namespace VulkanRHI
