#pragma once

#include "Engine/AssetRegistry/Asset.hxx"
#include "Engine/AssetRegistry/Material.hxx"
#include "Engine/Math/Transform.hxx"

namespace ecs
{

struct FMeshComponent {
    RTTI_DECLARE_TYPEINFO_MINIMAL(FMeshComponent);

public:
    Ref<RAsset> Asset;
    Ref<RMaterial> Material;

    FRHIGraphicsPipelineSpecification Pipeline;
};

template <typename T>
using TTransformComponent = Math::TTransform<T>;

using FTransformComponent = TTransformComponent<float>;

}    // namespace ecs
