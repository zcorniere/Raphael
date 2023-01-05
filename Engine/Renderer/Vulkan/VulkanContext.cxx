#include "Engine/Renderer/Vulkan/VulkanContext.hxx"

#include "Engine/Renderer/Vulkan/Vulkan.hxx"
#include "Engine/Renderer/Vulkan/VulkanAllocator.hxx"
#include <GLFW/glfw3.h>

#ifndef VK_API_VERSION_1_2
    #error Wrong Vulkan SDK !
#endif

namespace Raphael
{

#ifdef RPH_DEBUG
static bool s_Validation = true;
#else
static bool s_Validation = false;
#endif

constexpr static std::string_view VulkanMessageType(const VkDebugUtilsMessageTypeFlagsEXT &s)
{
    switch (s) {
        case 7: return "General | Validation | Performance";
        case 6: return "Validation | Performance";
        case 5: return "General | Performance";
        case 4 /*VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT*/: return "Performance";
        case 3: return "General | Validation";
        case 2 /*VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT*/: return "Validation";
        case 1 /*VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT*/: return "General";
        default: return "Unknown";
    }
}

constexpr cpplogger::Level VulkanMessageSeverity(const VkDebugUtilsMessageSeverityFlagBitsEXT severity)
{
    switch (severity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: return cpplogger::Level::Error;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: return cpplogger::Level::Warn;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: return cpplogger::Level::Info;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: return cpplogger::Level::Debug;
        default: return cpplogger::Level::Trace;
    }
}

static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugUtilsMessengerCallback(
    const VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, const VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *)
{
    const bool performanceWarnings = false;
    if (!performanceWarnings) {
        if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) return VK_FALSE;
    }

    std::string labels;
    std::string objects;
    if (pCallbackData->cmdBufLabelCount) {
        labels = fmt::format("\tLabels({}): \n", pCallbackData->cmdBufLabelCount);
        for (uint32_t i = 0; i < pCallbackData->cmdBufLabelCount; ++i) {
            const auto &label = pCallbackData->pCmdBufLabels[i];
            const std::string colorStr =
                fmt::format("[ {}, {}, {}, {} ]", label.color[0], label.color[1], label.color[2], label.color[3]);
            labels.append(fmt::format("\t\t- Command Buffer Label[{0}]: name: {1}, color: {2}\n", i,
                                      label.pLabelName ? label.pLabelName : "NULL", colorStr));
        }
    }
    if (pCallbackData->objectCount) {
        objects = fmt::format("\tObjects({}): \n", pCallbackData->objectCount);
        for (uint32_t i = 0; i < pCallbackData->objectCount; ++i) {
            const auto &object = pCallbackData->pObjects[i];
            objects.append(fmt::format("\t\t- Object[{0}] name: {1}, type: {2}, handle: {3:#x}\n", i,
                                       object.pObjectName ? object.pObjectName : "NULL",
                                       Utils::VkObjectTypeToString(object.objectType), object.objectHandle));
        }
    }
    LOG_V(Utils::LogVulkan, VulkanMessageSeverity(messageSeverity), "{} message:\n\t{}\n{} {}",
          VulkanMessageType(messageType), pCallbackData->pMessage, labels, objects);

    return VK_FALSE;
}

static bool CheckDriverAPIVersionSupport(uint32_t minimumSupportedVersion)
{
    uint32_t instanceVersion;
    vkEnumerateInstanceVersion(&instanceVersion);

    if (instanceVersion < minimumSupportedVersion) {
        LOG(Utils::LogVulkan, Fatal, "Incompatible Vulkan driver version!");
        LOG(Utils::LogVulkan, Fatal, "  You have {}.{}.{}", VK_API_VERSION_MAJOR(instanceVersion),
            VK_API_VERSION_MINOR(instanceVersion), VK_API_VERSION_PATCH(instanceVersion));
        LOG(Utils::LogVulkan, Fatal, "  You need at least {}.{}.{}", VK_API_VERSION_MAJOR(minimumSupportedVersion),
            VK_API_VERSION_MINOR(minimumSupportedVersion), VK_API_VERSION_PATCH(minimumSupportedVersion));

        return false;
    }

    return true;
}

static std::vector<const char *> GLFWgetRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    return {glfwExtensions, glfwExtensions + glfwExtensionCount};
}

DECLARE_LOGGER_CATEGORY(Core, LogVulkanContext, Info)

static bool checkIfValidationIsPresent(std::string_view validationLayerName)
{
    uint32_t instanceLayerCount;
    vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
    std::vector<VkLayerProperties> instanceLayerProperties(instanceLayerCount);
    vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayerProperties.data());
    LOG(LogVulkanContext, Trace, "Vulkan Instance Layers:");
    for (const VkLayerProperties &layer: instanceLayerProperties) {
        LOG(LogVulkanContext, Trace, "  {0}", layer.layerName);
        if (strcmp(layer.layerName, validationLayerName.data()) == 0) { return true; }
    }
    return false;
}

VulkanContext::VulkanContext()
{
}

VulkanContext::~VulkanContext()
{
    m_Device->Destroy();

    if (s_Validation) {
        auto vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            s_VulkanInstance, "vkDestroyDebugUtilsMessengerEXT");
        check(vkDestroyDebugUtilsMessengerEXT != nullptr);
        vkDestroyDebugUtilsMessengerEXT(s_VulkanInstance, m_DebugUtilsMessenger, nullptr);
    }

    vkDestroyInstance(s_VulkanInstance, nullptr);
    s_VulkanInstance = nullptr;
}

void VulkanContext::Init()
{
    LOG(LogVulkanContext, Info, "VulkanContext::Init");
    checkMsg(glfwVulkanSupported(), "GLFW must support Vulkan !");

    if (!CheckDriverAPIVersionSupport(VK_API_VERSION_1_2)) {
        checkMsg(false, "Incompatible Vulkan driver version.\nUpdate your GPU drivers!");
        std::terminate();
    }

    // Vulkan application info
    VkApplicationInfo appInfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Raphael",
        .pEngineName = "Raphael",
        .apiVersion = VK_API_VERSION_1_2,
    };

    // Extensions and Validation
    std::vector<const char *> instanceExtensions = GLFWgetRequiredExtensions();
    instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    if (s_Validation) {
        instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    VkValidationFeatureEnableEXT enables[] = {VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT};
    VkValidationFeaturesEXT features{
        .sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
        .enabledValidationFeatureCount = std::size(enables),
        .pEnabledValidationFeatures = enables,
    };

    VkInstanceCreateInfo instanceCreateInfo{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = (s_Validation) ? (&features) : (nullptr),
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = static_cast<std::uint32_t>(instanceExtensions.size()),
        .ppEnabledExtensionNames = instanceExtensions.data(),
    };

    if (s_Validation) {
        const char *validationLayerName = "VK_LAYER_KHRONOS_validation";
        // Check if this layer is available at instance level

        if (checkIfValidationIsPresent(validationLayerName)) {
            instanceCreateInfo.ppEnabledLayerNames = &validationLayerName;
            instanceCreateInfo.enabledLayerCount = 1;
        } else {
            LOG(LogVulkanContext, Error, "Validation layer {} not present, validation is disabled",
                validationLayerName);
        }
    }

    // Instance and surface Creation
    VK_CHECK_RESULT(vkCreateInstance(&instanceCreateInfo, nullptr, &s_VulkanInstance));
    Utils::VulkanLoadDebugUtilsExtensions(s_VulkanInstance);

    if (s_Validation) {
        auto vkCreateDebugUtilsMEssengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            s_VulkanInstance, "vkCreateDebugUtilsMessengerEXT");
        check(vkCreateDebugUtilsMEssengerEXT != nullptr);

        VkDebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .pNext = nullptr,
            .messageSeverity =
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = VulkanDebugUtilsMessengerCallback,
        };
        VK_CHECK_RESULT(
            vkCreateDebugUtilsMEssengerEXT(s_VulkanInstance, &debugUtilsCreateInfo, nullptr, &m_DebugUtilsMessenger));
    }

    m_PhysicalDevice = VulkanPhysicalDevice::Select();

    VkPhysicalDeviceFeatures enabledFeatures;
    memset(&enabledFeatures, 0, sizeof(VkPhysicalDeviceFeatures));
    enabledFeatures.samplerAnisotropy = true;
    enabledFeatures.wideLines = true;
    enabledFeatures.fillModeNonSolid = true;
    enabledFeatures.independentBlend = true;
    enabledFeatures.pipelineStatisticsQuery = true;
    m_Device = Ref<VulkanDevice>::Create(m_PhysicalDevice, enabledFeatures);

    VulkanAllocator::Init(m_Device);
}

}    // namespace Raphael
