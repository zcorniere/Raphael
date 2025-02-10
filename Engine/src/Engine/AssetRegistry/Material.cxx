#include "Engine/AssetRegistry/Material.hxx"

#include "Engine/Core/RHI/RHI.hxx"

RMaterial::RMaterial(FRHIGraphicsPipelineSpecification InPipeline): Pipeline(InPipeline)
{
    GraphicsPipeline = RHI::CreateGraphicsPipeline(Pipeline);
}

RMaterial::~RMaterial()
{
    GraphicsPipeline = nullptr;
}

void RMaterial::SetName(std::string_view Name)
{
    Super::SetName(Name);
    GraphicsPipeline->SetName(Name);
}

void RMaterial::SetInput(std::string_view Name, const Ref<RRHIBuffer>& Buffer)
{
    GraphicsPipeline->SetInput(Name, Buffer);
}

void RMaterial::Bind()
{
    ENQUEUE_RENDER_COMMAND(MaterialBind)
    ([this](FFRHICommandList& CommandList) { CommandList.SetPipeline(GraphicsPipeline); });
}

void RMaterial::Unbind()
{
}
