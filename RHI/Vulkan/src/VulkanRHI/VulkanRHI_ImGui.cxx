#include "VulkanRHI/VulkanRHI_ImGui.hxx"

#include "VulkanRHI/VulkanDevice.hxx"

#include "imgui_impl_vulkan.h"

namespace VulkanRHI
{

void VulkanRHI_ImGui::Initialize(FVulkanDevice* Device, ImGui_ImplVulkan_InitInfo InInitInfo)
{

    IMGUI_CHECKVERSION();
    ImGui_ImplVulkan_LoadFunctions(
        RHI_VULKAN_VERSION,
        [](const char* FunctionName, void* UserData) -> PFN_vkVoidFunction
        {
            VkInstance Instance = reinterpret_cast<VkInstance>(UserData);
            return VulkanAPI::vkGetInstanceProcAddr(Instance, FunctionName);
        },
        InInitInfo.Instance);

    InitInfo = InInitInfo;
    ImGui::CreateContext();

    CreateDescriptorPool(Device);

    InitInfo.DescriptorPool = DescriptorPool;
    ImGui_ImplVulkan_Init(&InitInfo);
}

void VulkanRHI_ImGui::Shutdown()
{
    ImGui_ImplVulkan_Shutdown();
    VulkanAPI::vkDestroyDescriptorPool(InitInfo.Device, DescriptorPool, VULKAN_CPU_ALLOCATOR);
    DescriptorPool = VK_NULL_HANDLE;
    ImGui::DestroyContext();
}

void VulkanRHI_ImGui::CreateDescriptorPool(FVulkanDevice* Device)
{
    const VkDescriptorPoolSize pool_sizes[]{
        {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000},
    };

    VkDescriptorPoolCreateInfo ImguiPoolCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = 1000,
        .poolSizeCount = std::size(pool_sizes),
        .pPoolSizes = pool_sizes,
    };
    VulkanAPI::vkCreateDescriptorPool(Device->GetHandle(), &ImguiPoolCreateInfo, VULKAN_CPU_ALLOCATOR, &DescriptorPool);
    VULKAN_SET_DEBUG_NAME(Device, VK_OBJECT_TYPE_DESCRIPTOR_POOL, DescriptorPool, "Imgui Descriptor Pool");
}

}    // namespace VulkanRHI
