#include "VulkanRHI/Resources/VulkanMaterial.hxx"

#include "VulkanRHI/Resources/VulkanGraphicsPipeline.hxx"
#include "VulkanRHI/VulkanCommandsObjects.hxx"

namespace VulkanRHI

{

RVulkanMaterial::RVulkanMaterial(FVulkanDevice* InDevice, WeakRef<RVulkanGraphicsPipeline> InPipeline)
    : IDeviceChild(InDevice)
    , Pipeline(InPipeline)
    , DescriptorManager(InDevice, Pipeline->GetShaders())
{
}

RVulkanMaterial::~RVulkanMaterial()
{
    DescriptorManager.Destroy();
}

void RVulkanMaterial::SetName(std::string_view InName)
{
    Super::SetName(InName);
    Pipeline->SetName(std::format("{:s} Pipeline", InName));
}

void RVulkanMaterial::Prepare()
{
    DescriptorManager.InvalidateAndUpdate();
}

void RVulkanMaterial::Bake()
{
    if (WasBaked())
    {
        return;
    }
    DescriptorManager.Bake();
}

bool RVulkanMaterial::WasBaked() const
{
    return DescriptorManager.GetHandle() != VK_NULL_HANDLE;
}

void RVulkanMaterial::SetInput(std::string_view Name, const Ref<RRHIBuffer>& Buffer)
{
    Ref<RVulkanBuffer> VulkanBuffer = Buffer.As<RVulkanBuffer>();
    DescriptorManager.SetInput(Name, VulkanBuffer);
}

void RVulkanMaterial::SetInput(std::string_view Name, const Ref<RRHITexture>& Buffer)
{
    Ref<RVulkanBuffer> VulkanBuffer = Buffer.As<RRHITexture>();
    DescriptorManager.SetInput(Name, VulkanBuffer);
}

}    // namespace VulkanRHI
