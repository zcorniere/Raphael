#pragma once

#include "Engine/AssetRegistry/Asset.hxx"
#include "Engine/Core/RHI/Resources/RHIMaterial.hxx"
#include "Engine/Math/Transform.hxx"

namespace ecs
{

struct FMeshComponent {
    RTTI_DECLARE_TYPEINFO_MINIMAL(FMeshComponent);

public:
    Ref<RAsset> Asset;
    Ref<RRHIMaterial> Material;
};

template <typename T>
using TTransformComponent = Math::TTransform<T>;

using FTransformComponent = TTransformComponent<float>;

}    // namespace ecs
