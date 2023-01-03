#include "Engine/Renderer/Vulkan/VulkanRenderer.hxx"

#include "Engine/Renderer/Vulkan/VulkanContext.hxx"

namespace Raphael
{
namespace Utils
{

    static constexpr std::string_view VulkanVendorIDToString(std::uint32_t vendorID)
    {
        switch (vendorID) {
            case 0x10DE: return "NVIDIA";
            case 0x1002: return "AMD";
            case 0x8086: return "INTEL";
        }
        return "Unknown";
    }

}    // namespace Utils

struct VulkanRendererData {
    RendererCapabilities RenderCapabilities;
};

static VulkanRendererData *s_Data = nullptr;

void VulkanRenderer::Init()
{
    s_Data = new VulkanRendererData();

    auto &caps = s_Data->RenderCapabilities;
    auto &properties = VulkanContext::GetCurrentDevice()->GetPhysicalDevice()->GetProperties();
    caps.Vendor = Utils::VulkanVendorIDToString(properties.vendorID);
    caps.Device = properties.deviceName;
    caps.Version = std::to_string(properties.driverVersion);

    Utils::DumpGPUInfo();
}

void VulkanRenderer::Shutdown()
{
    delete s_Data;
    s_Data = nullptr;
}

const RendererCapabilities &VulkanRenderer::GetCapabilities() const
{
    return s_Data->RenderCapabilities;
}

}    // namespace Raphael
