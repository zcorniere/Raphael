#pragma once

#include "Engine/AssetRegistry/Asset.hxx"
#include "Engine/GameFramework/Components/SceneComponent.hxx"

class RMeshComponent : public RSceneComponent
{
    RTTI_DECLARE_TYPEINFO(RMeshComponent, RSceneComponent)
public:
    RMeshComponent() = default;
    ~RMeshComponent() = default;

    void SetAsset(Ref<RAsset> InAsset);

public:
    Ref<RAsset> Asset = nullptr;
    Ref<RRHIMaterial> Material = nullptr;
};

inline void RMeshComponent::SetAsset(Ref<RAsset> InAsset)
{
    Asset = InAsset;
    MarkRenderStateDirty();
}
