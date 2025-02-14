#pragma once

#include "imgui_impl_vulkan.h"

class RWindow;

namespace VulkanRHI
{
class FVulkanDevice;

class VulkanRHI_ImGui
{
public:
    void Initialize(FVulkanDevice* Device, ImGui_ImplVulkan_InitInfo InitInfo);
    void Shutdown();

private:
    void CreateDescriptorPool(FVulkanDevice* Device);

private:
    ImGui_ImplVulkan_InitInfo InitInfo;
    VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;
};

}    // namespace VulkanRHI
