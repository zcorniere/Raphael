#include "Engine/Renderer/Vulkan/Linux/VulkanLinuxPlatform.hxx"
#include "Engine/Renderer/RHI/IDynamicRHI.hxx"

#include "Engine/Renderer/Vulkan/VulkanLoader.hxx"
#include "Engine/Renderer/Vulkan/VulkanUtils.hxx"

#include <GLFW/glfw3.h>
#include <dlfcn.h>

#define DEFINE_VK_ENTRYPOINTS(Type, Func) Type VulkanAPI::Func = NULL;
namespace Raphael
{
VK_ENTRYPOINT_ALL(DEFINE_VK_ENTRYPOINTS)
}
#undef DEFINE_VK_ENTRYPOINTS

namespace Raphael::RHI
{

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
#define CHECK_VK_ENTRYPOINTS(Type, Func)                            \
    if (VulkanAPI::Func == NULL) {                                  \
        bFoundAllEntryPoints = false;                               \
        LOG(LogRHI, Warn, "Failed to find entry point for " #Func); \
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
#define CHECK_VK_ENTRYPOINTS(Type, Func)                               \
    if (VulkanAPI::Func == NULL) {                                     \
        bFoundAllEntryPoints = false;                                  \
        LOG(LogRHI, Warn, "Failed to find entry point for {}", #Func); \
    }

#define GETINSTANCE_VK_ENTRYPOINTS(Type, Func) \
    VulkanAPI::Func = (Type)VulkanAPI::vkGetInstanceProcAddr(inInstance, #Func);

    VK_ENTRYPOINTS_INSTANCE(GETINSTANCE_VK_ENTRYPOINTS);
    VK_ENTRYPOINTS_INSTANCE(CHECK_VK_ENTRYPOINTS);
    VK_ENTRYPOINTS_SURFACE_INSTANCE(GETINSTANCE_VK_ENTRYPOINTS);
    VK_ENTRYPOINTS_SURFACE_INSTANCE(CHECK_VK_ENTRYPOINTS);

    if (!bFoundAllEntryPoints) { return false; }

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
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    for (unsigned i = 0; i < glfwExtensionCount; i++) {
        OutExtensions.push_back(glfwExtensions[i]);
    }
}

void VulkanLinuxPlatform::GetDeviceExtensions([[maybe_unused]] VulkanDevice *Device,
                                              [[maybe_unused]] std::vector<const char *> &OutExtensions)
{
}

void VulkanLinuxPlatform::CreateSurface(void *WindowHandle, VkInstance Instance, VkSurfaceKHR *OutSurface)
{
    check(WindowHandle);

    VK_CHECK_RESULT(glfwCreateWindowSurface(Instance, (GLFWwindow *)WindowHandle, nullptr, OutSurface));
}

}    // namespace Raphael::RHI
