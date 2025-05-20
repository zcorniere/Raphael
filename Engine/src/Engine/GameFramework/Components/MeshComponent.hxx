#pragma once

#include "Engine/AssetRegistry/Asset.hxx"
#include "Engine/GameFramework/Components/SceneComponent.hxx"

class RMeshComponent : public RSceneComponent
{
    DECLARE_RTTI(RMeshComponent);
    RTTI_DECLARE_TYPEINFO(RMeshComponent, RSceneComponent)
public:
    RMeshComponent() = default;
    ~RMeshComponent() = default;

    void SetAsset(Ref<RAsset> InAsset);
    void SetMaterial(Ref<RRHIMaterial> InMaterial);

public:
    Ref<RAsset> Asset = nullptr;
    Ref<RRHIMaterial> Material = nullptr;
};

RTTI_BEGIN_CLASS_DECLARATION(RMeshComponent)
PARENT_CLASS(RSceneComponent);
REF_PROPERTY(Asset);
RTTI_END_CLASS_DECLARATION;

inline void RMeshComponent::SetAsset(Ref<RAsset> InAsset)
{
    Asset = InAsset;
    MarkRenderStateDirty();
}

inline void RMeshComponent::SetMaterial(Ref<RRHIMaterial> InMaterial)
{
    Material = InMaterial;
    MarkRenderStateDirty();
}
