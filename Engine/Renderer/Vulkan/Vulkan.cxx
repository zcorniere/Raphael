#include "Engine/Renderer/Vulkan/Vulkan.hxx"

namespace Raphael::Utils
{

void VulkanLoadDebugUtilsExtensions(VkInstance instance)
{
    fpSetDebugUtilsObjectNameEXT =
        (PFN_vkSetDebugUtilsObjectNameEXT)(vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT"));
    if (fpSetDebugUtilsObjectNameEXT == nullptr)
        fpSetDebugUtilsObjectNameEXT = [](VkDevice, const VkDebugUtilsObjectNameInfoEXT *) { return VK_SUCCESS; };

    fpCmdBeginDebugUtilsLabelEXT =
        (PFN_vkCmdBeginDebugUtilsLabelEXT)(vkGetInstanceProcAddr(instance, "vkCmdBeginDebugUtilsLabelEXT"));
    if (fpCmdBeginDebugUtilsLabelEXT == nullptr)
        fpCmdBeginDebugUtilsLabelEXT = [](VkCommandBuffer, const VkDebugUtilsLabelEXT *) {};

    fpCmdEndDebugUtilsLabelEXT =
        (PFN_vkCmdEndDebugUtilsLabelEXT)(vkGetInstanceProcAddr(instance, "vkCmdEndDebugUtilsLabelEXT"));
    if (fpCmdEndDebugUtilsLabelEXT == nullptr) fpCmdEndDebugUtilsLabelEXT = [](VkCommandBuffer) {};

    fpCmdInsertDebugUtilsLabelEXT =
        (PFN_vkCmdInsertDebugUtilsLabelEXT)(vkGetInstanceProcAddr(instance, "vkCmdInsertDebugUtilsLabelEXT"));
    if (fpCmdInsertDebugUtilsLabelEXT == nullptr)
        fpCmdInsertDebugUtilsLabelEXT = [](VkCommandBuffer, const VkDebugUtilsLabelEXT *) {};
}

}    // namespace Raphael::Utils
