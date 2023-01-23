#include "Engine/Renderer/Vulkan/VulkanDevice.hxx"

namespace Raphael::RHI
{

VulkanDevice::VulkanDevice(VulkanDynamicRHI *InRHI, VkPhysicalDevice InGpu): Device(VK_NULL_HANDLE), Gpu(InGpu)
{
    std::memset(&GpuProps, 0, sizeof(GpuProps));
    std::memset(&PhysicalFeatures, 0, sizeof(PhysicalFeatures));
}

VulkanDevice::~VulkanDevice()
{
}

void VulkanDevice::InitGPU()
{
}

void VulkanDevice::CreateDevice(std::vector<const char *> DeviceLayers, std::vector<const char *> DeviceExtensions)
{
}

void VulkanDevice::PrepareForDestroy()
{
}

void VulkanDevice::Destroy()
{
}

void VulkanDevice::WaitUntilIdle()
{
}

}    // namespace Raphael::RHI
