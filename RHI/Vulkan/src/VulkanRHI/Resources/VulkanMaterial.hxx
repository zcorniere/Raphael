#pragma once

#include "Engine/Core/RHI/Resources/RHIMaterial.hxx"

#include "VulkanRHI/DescriptorPoolManager.hxx"

namespace VulkanRHI
{
class RVulkanGraphicsPipeline;

class RVulkanMaterial : public RRHIMaterial, public IDeviceChild
{
    RTTI_DECLARE_TYPEINFO(RVulkanMaterial, RRHIMaterial)
public:
    RVulkanMaterial(FVulkanDevice* InDevice, WeakRef<RVulkanGraphicsPipeline> Pipeline);
    virtual ~RVulkanMaterial();

    virtual void Prepare() override;
    virtual void Bake() override;
    virtual bool WasBaked() const override;

    virtual void SetInput(std::string_view Name, const Ref<RRHIBuffer>& Buffer) override;
    virtual void SetInput(std::string_view Name, const Ref<RRHITexture>& Texture) override;

    TArray<VkDescriptorSet> GetDescriptorSet() const
    {
        return DescriptorManager.GetDescriptorSets();
    }
    Ref<RVulkanGraphicsPipeline> GetPipeline() const
    {
        return Pipeline;
    }

private:
    Ref<RVulkanGraphicsPipeline> Pipeline;
    FDescriptorSetManager DescriptorManager;
};

}    // namespace VulkanRHI
