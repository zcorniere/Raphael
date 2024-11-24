#include "VulkanRHI/VulkanPlatform.hxx"
#include "VulkanRHI/VulkanLoader.hxx"
#include "VulkanRHI/VulkanRHI.hxx"
#include "VulkanRHI/VulkanUtils.hxx"

#include "Engine/Core/Window.hxx"
#include "Engine/Platforms/Platform.hxx"
#include "Engine/Platforms/PlatformMisc.hxx"

#include <GLFW/glfw3.h>

namespace VulkanRHI
{

#define DEFINE_VK_ENTRYPOINTS(Type, Func) Type VulkanAPI::Func = nullptr;
VK_ENTRYPOINT_ALL(DEFINE_VK_ENTRYPOINTS)
#undef DEFINE_VK_ENTRYPOINTS

static Ref<IExternalModule> s_VulkanModuleHandle = nullptr;

#if defined(PLATFORM_WINDOWS)
static constexpr auto VulkanLibraryName = "vulkan-1.dll";
#elif defined(PLATFORM_LINUX)
static constexpr auto VulkanLibraryName = "libvulkan.so.1";
#else
    #error "Unsupported Platform"
#endif

bool FVulkanPlatform::LoadVulkanLibrary()
{
    RPH_PROFILE_FUNC()

    if (s_VulkanModuleHandle) {
        return true;
    }

    s_VulkanModuleHandle = FPlatformMisc::LoadExternalModule(VulkanLibraryName);

    if (s_VulkanModuleHandle == nullptr) {
        return false;
    }

    bool bFoundAllEntryPoints = true;

#define GET_VK_ENTRYPOINTS(Type, Func) VulkanAPI::Func = s_VulkanModuleHandle->GetSymbol<Type>(#Func);
#define CHECK_VK_ENTRYPOINTS(Type, Func)                                   \
    if (VulkanAPI::Func == nullptr) {                                      \
        bFoundAllEntryPoints = false;                                      \
        LOG(LogVulkanRHI, Error, "Failed to find entry point for " #Func); \
    }

    VK_ENTRYPOINTS_BASE(GET_VK_ENTRYPOINTS);
    VK_ENTRYPOINTS_BASE(CHECK_VK_ENTRYPOINTS);
#undef GET_VK_ENTRYPOINTS
#undef CHECK_VK_ENTRYPOINTS

    if (!bFoundAllEntryPoints) {
        FreeVulkanLibrary();
        return false;
    }

    return true;
}

bool FVulkanPlatform::LoadVulkanInstanceFunctions(VkInstance inInstance)
{
    RPH_PROFILE_FUNC()

    bool bFoundAllEntryPoints = true;
#define CHECK_VK_ENTRYPOINTS(Type, Func)                                     \
    if (VulkanAPI::Func == nullptr) {                                        \
        bFoundAllEntryPoints = false;                                        \
        LOG(LogVulkanRHI, Warning, "Failed to find entry point for " #Func); \
    }

#define GETINSTANCE_VK_ENTRYPOINTS(Type, Func) \
    VulkanAPI::Func = reinterpret_cast<Type>(VulkanAPI::vkGetInstanceProcAddr(inInstance, #Func));

    VK_ENTRYPOINTS_INSTANCE(GETINSTANCE_VK_ENTRYPOINTS);
    VK_ENTRYPOINTS_INSTANCE(CHECK_VK_ENTRYPOINTS);
    VK_ENTRYPOINTS_SURFACE_INSTANCE(GETINSTANCE_VK_ENTRYPOINTS);
    VK_ENTRYPOINTS_SURFACE_INSTANCE(CHECK_VK_ENTRYPOINTS);
    VK_ENTRYPOINTS_DEBUG_UTILS(GETINSTANCE_VK_ENTRYPOINTS);
    VK_ENTRYPOINTS_DEBUG_UTILS(CHECK_VK_ENTRYPOINTS);

#undef GETINSTANCE_VK_ENTRYPOINTS
#undef CHECK_VK_ENTRY_POINTS

    return bFoundAllEntryPoints;
}

void FVulkanPlatform::FreeVulkanLibrary()
{
    if (s_VulkanModuleHandle != nullptr) {
#define CLEAR_VK_ENTRYPOINTS(Type, Func) VulkanAPI::Func = nullptr;
        VK_ENTRYPOINT_ALL(CLEAR_VK_ENTRYPOINTS);
#undef CLEAR_VK_ENTRYPOINTS

        s_VulkanModuleHandle = nullptr;
    }
}

void FVulkanPlatform::CreateSurface(RWindow* WindowHandle, VkInstance Instance, VkSurfaceKHR* OutSurface)
{
    RWindow::EnsureGLFWInit();

    VK_CHECK_RESULT(glfwCreateWindowSurface(Instance, WindowHandle->GetHandle(), VULKAN_CPU_ALLOCATOR, OutSurface));
}

}    // namespace VulkanRHI
