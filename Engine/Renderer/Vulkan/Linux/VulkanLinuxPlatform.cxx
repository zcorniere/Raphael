#include "Engine/Renderer/Vulkan/Linux/VulkanLinuxPlatform.hxx"

#include "Engine/Renderer/RHI/IDynamicRHI.hxx"
#include "Engine/Renderer/Vulkan/VulkanLoader.hxx"
#include "Engine/Renderer/Vulkan/VulkanUtils.hxx"

#include "Engine/Platforms/Window.hxx"

#include <SDL.h>
#include <SDL_vulkan.h>

#include <dlfcn.h>

#define DEFINE_VK_ENTRYPOINTS(Type, Func) Type VulkanAPI::Func = NULL;

VK_ENTRYPOINT_ALL(DEFINE_VK_ENTRYPOINTS)
VK_ENTRYPOINTS_DEBUG_UTILS(DEFINE_VK_ENTRYPOINTS)

#undef DEFINE_VK_ENTRYPOINTS

DECLARE_LOGGER_CATEGORY(Core, LogVulkanLinux, Info)

void *VulkanLinuxPlatform::VulkanLib = nullptr;
bool VulkanLinuxPlatform::bAttemptedLoad = false;

bool VulkanLinuxPlatform::LoadVulkanLibrary()
{
    if (bAttemptedLoad) { return VulkanLib != nullptr; }
    bAttemptedLoad = true;

    // open libvulkan.so
    VulkanLib = dlopen("libvulkan.so.1", RTLD_NOW | RTLD_LOCAL);
    if (VulkanLib == nullptr) { return false; }

    bool bFoundAllEntryPoints = true;
#define CHECK_VK_ENTRYPOINTS(Type, Func)                                    \
    if (VulkanAPI::Func == NULL) {                                          \
        bFoundAllEntryPoints = false;                                       \
        LOG(LogVulkanLinux, Warn, "Failed to find entry point for " #Func); \
    }
#define GET_VK_ENTRYPOINTS(Type, Func) VulkanAPI::Func = (Type)dlsym(VulkanLib, #Func);
    VK_ENTRYPOINTS_BASE(GET_VK_ENTRYPOINTS);
    VK_ENTRYPOINTS_BASE(CHECK_VK_ENTRYPOINTS);

    if (!bFoundAllEntryPoints) {
        dlclose(VulkanLib);
        VulkanLib = nullptr;
        return false;
    }

    VK_ENTRYPOINTS_OPTIONAL_BASE(GET_VK_ENTRYPOINTS);
    VK_ENTRYPOINTS_OPTIONAL_BASE(CHECK_VK_ENTRYPOINTS);

    VK_ENTRYPOINTS_PLATFORM_BASE(GET_VK_ENTRYPOINTS);
    VK_ENTRYPOINTS_PLATFORM_BASE(CHECK_VK_ENTRYPOINTS);

#undef CHECK_VK_ENTRYPOINTS
#undef GET_VK_ENTRYPOINTS
    return true;
}

bool VulkanLinuxPlatform::LoadVulkanInstanceFunctions(VkInstance inInstance)
{
    bool bFoundAllEntryPoints = true;
#define CHECK_VK_ENTRYPOINTS(Type, Func)                                       \
    if (VulkanAPI::Func == NULL) {                                             \
        bFoundAllEntryPoints = false;                                          \
        LOG(LogVulkanLinux, Warn, "Failed to find entry point for {}", #Func); \
    }

#define GETINSTANCE_VK_ENTRYPOINTS(Type, Func) \
    VulkanAPI::Func = (Type)VulkanAPI::vkGetInstanceProcAddr(inInstance, #Func);

    VK_ENTRYPOINTS_INSTANCE(GETINSTANCE_VK_ENTRYPOINTS);
    VK_ENTRYPOINTS_INSTANCE(CHECK_VK_ENTRYPOINTS);
    VK_ENTRYPOINTS_SURFACE_INSTANCE(GETINSTANCE_VK_ENTRYPOINTS);
    VK_ENTRYPOINTS_SURFACE_INSTANCE(CHECK_VK_ENTRYPOINTS);

    if (!bFoundAllEntryPoints) { return false; }

    VK_ENTRYPOINTS_DEBUG_UTILS(GETINSTANCE_VK_ENTRYPOINTS);

    VK_ENTRYPOINTS_PLATFORM_INSTANCE(GETINSTANCE_VK_ENTRYPOINTS);
    VK_ENTRYPOINTS_PLATFORM_INSTANCE(CHECK_VK_ENTRYPOINTS);

#undef GETINSTANCE_VK_ENTRYPOINTS
#undef CHECK_VK_ENTRY_POINTS
    return true;
}

void VulkanLinuxPlatform::FreeVulkanLibrary()
{
    if (VulkanLib != nullptr) {
#define CLEAR_VK_ENTRYPOINTS(Type, Func) VulkanAPI::Func = nullptr;
        VK_ENTRYPOINT_ALL(CLEAR_VK_ENTRYPOINTS);
#undef CLEAR_VK_ENTRYPOINTS

        dlclose(VulkanLib);
        VulkanLib = nullptr;
    }
    bAttemptedLoad = false;
}

void VulkanLinuxPlatform::GetInstanceExtensions([[maybe_unused]] std::vector<const char *> &OutExtensions)
{
    Window::EnsureSDLInit();

    const char *SDLDriver = SDL_GetCurrentVideoDriver();
    check(SDLDriver);

    if (strcmp(SDLDriver, "x11") == 0) {
        OutExtensions.push_back("VK_KHR_xlib_surface");
    } else if (strcmp(SDLDriver, "wayland") == 0) {
        OutExtensions.push_back("VK_KHR_wayland_surface");
    } else {
        checkNoEntry();
    }
}

void VulkanLinuxPlatform::GetDeviceExtensions([[maybe_unused]] VulkanDevice *Device,
                                              [[maybe_unused]] std::vector<const char *> &OutExtensions)
{
}

void VulkanLinuxPlatform::CreateSurface(void *WindowHandle, VkInstance Instance, VkSurfaceKHR *OutSurface)
{
    Window::EnsureSDLInit();

    if (SDL_Vulkan_CreateSurface((SDL_Window *)WindowHandle, Instance, OutSurface) == SDL_FALSE) {
        LOG(LogVulkanLinux, Fatal, "Error initializing SDL Vulkan Surface: {}", SDL_GetError());
        checkNoEntry();
    }
}
